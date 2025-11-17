/*
// Copyright (c) 2025 MiTAC Computing Technology Corp.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#include <boost/program_options.hpp>
#include <boost/asio/steady_timer.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/property.hpp>
#include <xyz/openbmc_project/State/Host/server.hpp>
#include <xyz/openbmc_project/State/OperatingSystem/Status/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/Asset/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/UniqueIdentifier/server.hpp>

#include <fstream>
#include <iostream>

#include "mitac-dcm-telemetry.hpp"
#include "mrd_dbus_reg_intf.hpp"

#define RENEW_MRD_WDT_TIMEOUT 120

using Json = nlohmann::json;
using namespace boost::program_options;
using namespace sdbusplus::server::xyz::openbmc_project;
namespace fs = std::filesystem;

namespace dcm
{
namespace telemetry
{
using HostStateHandler = std::function<void(HostState, HostState)>;
using PowerState = state::Host::HostState;
using OsState = state::operating_system::Status::OSStatus;

MRDLoader* pMRDLoader = nullptr;
MRDDbusRegIntf* pMRDDbusIntf = nullptr; 

static boost::asio::io_context io;
static std::shared_ptr<sdbusplus::asio::connection> conn;

static boost::asio::steady_timer renewMRDtimer(io);

HostState hostState = HostState::off;
static PowerState powerState = PowerState::Off;
static OsState osState = OsState::Inactive;
static bool biosDone = false;
static std::vector<HostStateHandler> hostStateCallbacks;

void addHostStateCallback(HostStateHandler cb)
{
    hostStateCallbacks.push_back(cb);
}

static void updateHostState()
{
    HostState prevState = hostState;
    if (powerState == PowerState::Off)
    {
        hostState = HostState::off;
        // Make sure that we don't inadvertently jump back to PostComplete if
        // the HW status happens to turn back on before the biosDone goes false,
        // since the two signals come from different services and there is no
        // tight guarantee about their relationship.
        biosDone = false;
        // Setting osState to inactive for the same reason as above.
        osState = OsState::Inactive;
    }
    // Both biosDone and OsState tell us about the POST done status. At least
    // one of them should indicate that the POST is done.
    // According to openbmc_project/State/OperatingSystem/Status.interface.yaml
    // Only "Inactive" indicates that the POST is not done. All the other
    // statuses (CBoot, PXEBoot, DiagBoot, CDROMBoot, ROMBoot, BootComplete,
    // Standby) indicate that the POST is done.
    else if ((!biosDone) && (osState == OsState::Inactive))
    {
        hostState = HostState::postInProgress;
    }
    else
    {
        hostState = HostState::postComplete;
    }
    //DEBUG_PRINT << "new host state: " << static_cast<int>(hostState) << "\n";

    if (prevState != hostState)
    {
        for (const auto& cb : hostStateCallbacks)
        {
            cb(prevState, hostState);
        }
    }
}

void updatePowerState(const std::string& newState)
{
    powerState = state::Host::convertHostStateFromString(newState);
    updateHostState();
}

void updateBiosDone(bool newState)
{
    biosDone = newState;
    updateHostState();
}

void updateOsState(const std::string& newState)
{
    // newState might not contain the full path. It might just contain the enum
    // string (By the time I am writing this, its not returning the full path).
    // Full string:
    // "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Standby". Just
    // the string for enum: "Standby". If the newState doesn't contain the full
    // string, convertOSStatusFromString will fail. Prepend the full path if
    // needed.
    std::string full_path = newState;
    if (newState.find("xyz.") == std::string::npos)
    {
        full_path =
            "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus." +
            newState;
    }

    try
    {
        osState = state::operating_system::Status::convertOSStatusFromString(full_path);
    }
    catch (const sdbusplus::exception::InvalidEnumString& ex)
    {
        lg2::error("Exception: {WHAT}", "WHAT", ex.what());
        osState = OsState::Inactive;
    }
    lg2::info("OperatingSystem Status Changed: {STATUS}", "STATUS", full_path);
    updateHostState();
}

/**
 * Register a handler to be called whenever the given property is changed. Also
 * call the handler once immediately (asynchronously) with the current property
 * value.
 *
 * Since this necessarily reads all properties in the given interface, type
 * information about the interface may need to be provided via
 * CustomVariantArgs.
 *
 * @tparam  CustomVariantTypes  Any property types contained in the interface
 *                              beyond the base data types (numeric and
 *                              string-like types) and Handler's param type.
 * @tparam  Handler     Automatically deduced. Must be a callable taking a
 *                      single parameter whose type matches the property.
 *
 * @param[in]   service     D-Bus service name.
 * @param[in]   object      D-Bus object name.
 * @param[in]   interface   D-Bus interface name.
 * @param[in]   propertyName    D-Bus property name.
 * @param[in]   handler     Callable to be called immediately and upon any
 *                          changes in the property value.
 * @param[out]  propertiesChangedMatch  Optional pointer to receive a D-Bus
 *                                      match object, if you need to manage its
 *                                      lifetime.
 * @param[out]  interfacesAddedMatch    Optional pointer to receive a D-Bus
 *                                      match object, if you need to manage its
 *                                      lifetime.
 */
template <typename... CustomVariantTypes, typename Handler>
static void subscribeToProperty(
    const char* service, const char* object, const char* interface,
    const char* propertyName, Handler&& handler,
    sdbusplus::bus::match_t** propertiesChangedMatch = nullptr,
    sdbusplus::bus::match_t** interfacesAddedMatch = nullptr)
{
    // Type of first parameter to Handler, with const/& removed
    using PropertyType = std::remove_const_t<std::remove_reference_t<
        std::tuple_element_t<0, boost::callable_traits::args_t<Handler>>>>;
    // Base data types which we can handle by default
    using InterfaceVariant = typename sdbusplus::utility::dedup_variant_t<
        PropertyType, CustomVariantTypes..., bool, uint8_t, uint16_t, int16_t,
        uint32_t, int32_t, uint64_t, int64_t, size_t, ssize_t, double,
        std::string, sdbusplus::message::object_path>;

    sdbusplus::asio::getProperty<PropertyType>(
        *conn, service, object, interface, propertyName,
        [handler, propertyName = std::string(propertyName)](
            boost::system::error_code ec, const PropertyType& newValue) {
        if (ec)
        {
            lg2::error("Failed to read {PROPERTY_NAME}: {EC}", "PROPERTY_NAME", propertyName, "EC", ec.category().name());
            return;
        }
        handler(newValue);
    });

    using ChangedPropertiesType =
        std::vector<std::pair<std::string, InterfaceVariant>>;

    // Define some logic which is common to the two match callbacks, since they
    // both have to loop through all the properties in the interface.
    auto commonPropHandler = [propertyName = std::string(propertyName),
                              handler = std::forward<Handler>(handler)](
                                 const ChangedPropertiesType& changedProps) {
        for (const auto& [changedProp, newValue] : changedProps)
        {
            if (changedProp == propertyName)
            {
                const auto* actualVal = std::get_if<PropertyType>(&newValue);
                if (actualVal != nullptr)
                {
                    lg2::info("Property {PROPERTY_NAME} changed to {VALUE}", "PROPERTY_NAME", propertyName, "VALUE", *actualVal);
                    handler(*actualVal);
                }
                else
                {
                    lg2::error("Property {PROPERTY_NAME} had unexpected type", "PROPERTY_NAME", propertyName);
                }
                break;
            }
        }
    };

    // Set up a match for PropertiesChanged signal
    auto* propMatch = new sdbusplus::bus::match_t(
        *conn,
        sdbusplus::bus::match::rules::sender(service) +
            sdbusplus::bus::match::rules::propertiesChanged(object, interface),
        [commonPropHandler](sdbusplus::message_t& reply) {
        ChangedPropertiesType changedProps;
        // ignore first param (interface name), it has to be correct
        reply.read(std::string(), changedProps);
        lg2::debug("PropertiesChanged handled");
        commonPropHandler(changedProps);
    });

    // Set up a match for the InterfacesAdded signal from the service's
    // ObjectManager. This is useful in the case where the object is not added
    // yet, and when it's added they choose to not emit PropertiesChanged. So in
    // order to see the initial value when it comes, we need to watch this too.
    auto* intfMatch = new sdbusplus::bus::match_t(
        *conn,
        sdbusplus::bus::match::rules::sender(service) +
            sdbusplus::bus::match::rules::interfacesAdded(),
        [object = std::string(object), interface = std::string(interface),
         commonPropHandler](sdbusplus::message_t& reply) {
        sdbusplus::message::object_path changedObject;
        reply.read(changedObject);
        if (changedObject != object)
        {
            return;
        }

        std::vector<std::pair<std::string, ChangedPropertiesType>>
            changedInterfaces;
        reply.read(changedInterfaces);

        for (const auto& [changedInterface, changedProps] : changedInterfaces)
        {
            if (changedInterface != interface)
            {
                continue;
            }
            lg2::debug("InterfacesAdded handled");
            commonPropHandler(changedProps);
        }
    });

    if (propertiesChangedMatch != nullptr)
    {
        *propertiesChangedMatch = propMatch;
    }

    if (interfacesAddedMatch != nullptr)
    {
        *interfacesAddedMatch = intfMatch;
    }
}

static void wdtResubscribeMRD(const boost::system::error_code& ec)
{
    if (ec)
    {
        if (ec == boost::asio::error::operation_aborted)
        {
            lg2::warning("wdtResubscribeMRD timer aborted or rescheduled.");
            return;
        }
        else
        {
            lg2::error("wdtResubscribeMRD timer failed");
            return;
        }
    }
    lg2::info("wdtResubscribeMRD: timer expired");
    if (pMRDDbusIntf && pMRDLoader)
        pMRDLoader->resubscribeMRD(*pMRDDbusIntf);
}

static void hostStateHandler(HostState prevState, HostState currentState)
{
    switch (currentState)
    {
        case HostState::postComplete:
        {
            //Virtual sensors might activated after BIOS POST completed.
            //Reduced the waiting time when postComplete.
            const uint16_t timeout = RENEW_MRD_WDT_TIMEOUT/2;
            lg2::info("resubscribeMRD from postComplete after {TIMEOUT}sec.", "TIMEOUT", std::to_string(timeout));
            renewMRDtimer.expires_after(std::chrono::seconds(timeout));
            renewMRDtimer.async_wait(wdtResubscribeMRD);
            break;
        }
        case HostState::off:
            break;
        case HostState::postInProgress:
        {
            //Most of sensors activated after powered-on.
            //Added timer here for sensor discovery time.
            const uint16_t timeout = RENEW_MRD_WDT_TIMEOUT;
            lg2::info("resubscribeMRD from postInProgress after {TIMEOUT}sec.", "TIMEOUT", std::to_string(timeout));
            renewMRDtimer.expires_after(std::chrono::seconds(timeout));
            renewMRDtimer.async_wait(wdtResubscribeMRD);
            break;
        }
        default:
            lg2::warning("Unsupported HostState");
    } 
}

bool addMetricReportDef(const std::string id, const std::string jsonlized)
{
    return true;
}

bool removeMetricReportDef(const std::string id)
{
    return true;
}

MRDLoader::MRDLoader():mitacMRD(nullptr)
{
    lg2::info("Construct MRDLoader without MitacMDR config");
}

MRDLoader::MRDLoader(const std::string config):mitacMRD(nullptr)
{
    lg2::info("Construct MRDLoader with MitacMDR config");
    parseMRDConfig(config);
}

bool MRDLoader::parseMRDConfig(const std::string config)
{
    if (fs::exists(config.c_str()))
    {
        std::ifstream file;
        try
        {
            file.open(config.c_str());
            Json jsonConfig = Json::parse(file);
            file.close();
            if (validateMRD(&jsonConfig) == false) {
                lg2::error("Invalid format of MitacMRD config. {PATH}", "PATH", config);
                return false;
            }
            mitacMRD = jsonConfig; 
            return true;
        }
        catch (const std::exception& ex)
        {
            lg2::error("Exception: {WHAT}", "WHAT", ex.what());
            return false;
        }
    }
    else
    {
        lg2::error("Can't find MitacMRD config. {PATH}", "PATH", config);
        return false;
    }
}

bool MRDLoader::validateMRD(const nlohmann::json* jRoot)
{
    lg2::info("Enter validateMRD");
    if (jRoot == nullptr)
    {
        lg2::error("jRoot is NULL");
        return false;
    }    

    if (! (jRoot->contains("MitacMRD") && jRoot->at("MitacMRD").is_array()))
    {
        lg2::error("MRD config file shall contain MitacMRD json array.");
        return false;
    }

    Json mrd_array = jRoot->at("MitacMRD");
    // iterate the MitacMRD array
    bool result = true;
    for (Json::iterator it = mrd_array.begin(); it != mrd_array.end(); ++it) {
        if (! ((*it).is_object() && (*it).at("Id").is_string()))
        {
            lg2::error("Each element in MitacMRD shall contain Id property. {ENTRY}", "ENTRY", *it);
            result = false;
        }
    } 
    lg2::info("subscribeMRD: Result = {RESULT}", "RESULT", result);
    return result;
}

bool MRDLoader::subscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf)
{
    lg2::info("Enter subscribeMRD");
    if (mitacMRD == nullptr)
    {
        lg2::error("mrdRegIntf is NULL");
        return false;
    }    

    //Refresh available sensors
    mrdRegIntf.refreshSensorList();    

    Json mrd_array = mitacMRD.at("MitacMRD");
    for (Json::iterator it = mrd_array.begin(); it != mrd_array.end(); ++it) {
        mrdRegIntf.addMRD(*it);
    } 
    return true; 
}

bool MRDLoader::unsubscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf)
{
    lg2::info("Enter unsubscribeMRD");
    if (mitacMRD == nullptr)
    {
        lg2::error("mrdRegIntf is NULL");
        return false;
    }    
    
    Json mrd_array = mitacMRD.at("MitacMRD");
    for (Json::iterator it = mrd_array.begin(); it != mrd_array.end(); ++it) {
        Json mrd_entry = *it;
        std::string Id = mrd_entry.at("Id");
        mrdRegIntf.delMRD(Id);
    } 
    return true; 
}

bool MRDLoader::resubscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf)
{
    lg2::info("Enter resubscribeMRD");
    if (mitacMRD == nullptr)
    {
        lg2::error("mrdRegIntf is NULL");
        return false;
    }    
    mrdRegIntf.refreshSensorList();    
    
    Json mrd_array = mitacMRD.at("MitacMRD");
    for (Json::iterator it = mrd_array.begin(); it != mrd_array.end(); ++it) {
        Json mrd_entry = *it;
        std::string Id = mrd_entry.at("Id");
        mrdRegIntf.delMRD(Id);
        //std::this_thread::sleep_for(std::chrono::seconds(0.2));
        mrdRegIntf.addMRD(*it);
    } 
    return true; 
}

} //namespace telemetry
} // namespace dcm

int main(int argc, const char *argv[])
{
    using namespace dcm::telemetry;
    conn = std::make_shared<sdbusplus::asio::connection>(io);

    try
    {
        // Add options
        options_description desc{"Options"};
        desc.add_options()
            ("config", value<std::string>(), "Config file");
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("config"))
        {
            const std::string config_fn = vm["config"].as<std::string>();
            try
            {
                pMRDLoader = new MRDLoader(config_fn);
                pMRDDbusIntf = new dcm::telemetry::MRDDbusRegIntf(conn);
                try
                {
                    lg2::info("Try to subscrite property change for CoreBiosDone and OperatingSystemState.");
                    subscribeToProperty("xyz.openbmc_project.State.Host",
                                        "/xyz/openbmc_project/state/host0",
                                        state::Host::interface, "CurrentHostState",
                                        updatePowerState);
                    subscribeToProperty("xyz.openbmc_project.Host.Misc.Manager",
                                        "/xyz/openbmc_project/misc/platform_state",
                                        "xyz.openbmc_project.State.Host.Misc", "CoreBiosDone",
                                        updateBiosDone);
                    subscribeToProperty("xyz.openbmc_project.State.Host0",
                                        "/xyz/openbmc_project/state/host0",
                                        "xyz.openbmc_project.State.OperatingSystem.Status", "OperatingSystemState",
                                        updateOsState);

                    addHostStateCallback(hostStateHandler);
                    lg2::info("Start to monitor dbus event.");
                    io.run();
                    lg2::info("Delect objects");
                    delete pMRDDbusIntf;
                    delete pMRDLoader;
                    return 0;
                }
                catch (const error &ex)
                { 
                    lg2::error("Exception: {WHAT}", "WHAT", ex.what());
                }
            }
            catch (const error &ex)
            { 
                lg2::error("Exception: {WHAT}", "WHAT", ex.what());
            }
        }
    }
    catch (const error &ex)
    {
        lg2::error("Exception: {WHAT}", "WHAT", ex.what());
    }
}

