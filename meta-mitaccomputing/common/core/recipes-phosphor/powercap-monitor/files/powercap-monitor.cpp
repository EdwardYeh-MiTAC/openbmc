#include "powercap-monitor.hpp"
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/message.hpp>
#include <cstdlib>
#include <sstream>
#include <array>
#include <regex>
#include <thread>
#include <chrono>

namespace phosphor
{
namespace powercap
{

namespace
{
constexpr const char* OS_STATUS_STANDBY = "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Standby";
constexpr const char* OS_STATUS_INACTIVE = "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Inactive";

bool readApmlPower(int cpuId)
{
    constexpr int maxRetries = 3;
    constexpr int retryDelayMs = 100;

    for (int attempt = 1; attempt <= maxRetries; ++attempt)
    {
        std::stringstream cmd;
        cmd << "apml_tool " << cpuId << " -p";

        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe)
        {
            lg2::error("Failed to run APML tool for CPU{CPU}, attempt {TRY}",
                       "CPU", cpuId, "TRY", attempt);
            continue;
        }

        std::array<char, 512> buffer{};
        std::string output;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            output += buffer.data();
        }
        pclose(pipe);

        /* Output example
        ================================= APML System Management Interface ====================================
        ---------------------------------------------
        | Power (Watts)          | 55.976           |
        | PowerLimit (Watts)     | 210.000          |
        | PowerLimitMax (Watts)  | 240.000          |
        ---------------------------------------------
        ========================================== End of APML SMI ============================================
        */

        std::smatch match;
        std::regex pattern(R"(\|\s*Power\s*\(Watts\)\s*\|\s*([0-9]+\.[0-9]+)\s*\|)");

        if (std::regex_search(output, match, pattern) && match.size() > 1)
        {
            std::string powerStr = match[1];
            lg2::info("CPU{CPU} Power read successful: {W}W", "CPU", cpuId, "W", powerStr);
            return true;
        }
        else
        {
            lg2::warning("CPU{CPU} APML read failed or unexpected format (attempt {TRY})",
                         "CPU", cpuId, "TRY", attempt);
            lg2::warning("Raw output:\n{OUT}", "OUT", output);
        }

        if (attempt < maxRetries)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        }
    }

    lg2::info("CPU{CPU} APML read failed after {MAX} attempts",
               "CPU", cpuId, "MAX", maxRetries);
    lg2::info("CPU{CPU} might not be present (not detected by APML)", "CPU", cpuId);
    return false;
}

void writePowerCapToCpu(int cpuId, uint32_t capValue)
{
    std::stringstream cmd;
    cmd << "apml_tool " << cpuId << " -s " << (capValue * 1000);
    lg2::info("Executing: {CMD}", "CMD", cmd.str());
    std::system(cmd.str().c_str());
}
} // namespace

PowerCapMonitor::PowerCapMonitor(sdbusplus::bus_t& bus) :
    bus(bus),
    matchPowerCap(bus,
                  "type='signal',member='PropertiesChanged',"
                  "interface='org.freedesktop.DBus.Properties',"
                  "path='/xyz/openbmc_project/control/host0/power_cap'",
                  std::bind(&PowerCapMonitor::onPowerCapChanged, this, std::placeholders::_1)),
    matchOsState(bus,
                 "type='signal',member='PropertiesChanged',"
                 "interface='org.freedesktop.DBus.Properties',"
                 "path='/xyz/openbmc_project/state/host0'",
                 std::bind(&PowerCapMonitor::onOsStateChanged, this, std::placeholders::_1)),
    cpu0Ok(false),
    cpu1Ok(false)
{
    lg2::info("PowerCapMonitor initialized and monitoring DBus for changes");
}

void PowerCapMonitor::onPowerCapChanged(sdbusplus::message_t& msg)
{
    std::string interface;
    std::map<std::string, std::variant<uint32_t>> props;
    std::vector<std::string> invalidated;
    msg.read(interface, props, invalidated);

    auto it = props.find("PowerCap");
    if (it == props.end())
    {
        lg2::debug("PowerCap not present in PropertiesChanged");
        return;
    }

    if (const auto* value = std::get_if<uint32_t>(&it->second))
    {
        lg2::info("PowerCap changed to {VALUE}", "VALUE", *value);

        int cpuCount = 0;
        if (cpu0Ok) cpuCount++;
        if (cpu1Ok) cpuCount++;

        if (cpuCount == 0)
        {
            lg2::error("No available CPU detected for setting PowerCap");
            return;
        }

        uint32_t capValue = *value;
        if (cpuCount == 2)
        {
            capValue /= 2;
            lg2::info("Detected 2 CPUs, each set to {VAL}", "VAL", capValue);
        }
        else
        {
            lg2::info("Single CPU, setting value: {VAL}", "VAL", capValue);
        }

        if (cpu0Ok) writePowerCapToCpu(0, capValue);
        if (cpu1Ok) writePowerCapToCpu(1, capValue);
    }
    else
    {
        lg2::error("Failed to extract PowerCap value");
    }
}

void PowerCapMonitor::onOsStateChanged(sdbusplus::message_t& msg)
{
    std::string interface;
    std::map<std::string, std::variant<std::string>> props;
    std::vector<std::string> invalidated;
    msg.read(interface, props, invalidated);

    auto it = props.find("OperatingSystemState");
    if (it == props.end())
        return;

    if (const auto* value = std::get_if<std::string>(&it->second))
    {
        if (*value == OS_STATUS_STANDBY)
        {
            lg2::info("Detected POST Complete");

            cpu0Ok = readApmlPower(0);
            cpu1Ok = readApmlPower(1);

            lg2::info("CPU0 success: {CPU0}, CPU1 success: {CPU1}",
                      "CPU0", cpu0Ok, "CPU1", cpu1Ok);

            if (!cpu0Ok)
            {
                lg2::warning("CPU0 not available, skipping PowerCapEnable=true");
                return;
            }

            setPowerCapEnable(true);
        }
        else if (*value == OS_STATUS_INACTIVE)
        {
            lg2::info("Detected POST Complete DeAssert, OS state is turned to OFF");
            setPowerCapEnable(false);
        }
        else
        {
            lg2::debug("Unhandled OS state: {STATE}", "STATE", *value);
        }
    }
}

void PowerCapMonitor::setPowerCapEnable(bool enable)
{
    std::string targetService = "xyz.openbmc_project.Settings";
    std::string targetPath = "/xyz/openbmc_project/control/host0/power_cap";
    std::string targetInterface = "xyz.openbmc_project.Control.Power.Cap";
    std::string targetProperty = "PowerCapEnable";

    auto setMsg = bus.new_method_call(targetService.c_str(),
                                      targetPath.c_str(),
                                      "org.freedesktop.DBus.Properties",
                                      "Set");

    setMsg.append(targetInterface, targetProperty, std::variant<bool>(enable));

    try
    {
        bus.call(setMsg);
        lg2::info("PowerCapEnable set to {VAL}", "VAL", enable);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to set PowerCapEnable: {ERROR}", "ERROR", e.what());
    }
}

} // namespace powercap
} // namespace phosphor
