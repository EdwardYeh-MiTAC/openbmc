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
#include "ep-mgr.hpp"
#include "mctp.hpp"

#include <phosphor-logging/lg2.hpp>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>

using Json = nlohmann::json;
namespace fs = std::filesystem;
using namespace com::mitac_computing::mctp;
using namespace com::mitac_computing::utility;
/* MCTPEndPoint
*
*/

MCTPEndPoint::MCTPEndPoint(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid)
    :MCTPEndPointBase(epContext, net, eid), _EPHostStateObserver(nullptr)
{

}

bool MCTPEndPoint::createHostStateObserver(HostStateSubject* hostStateSubject) {
    if ((_EPHostStateObserver != nullptr) || (hostStateSubject == nullptr)) {
        lg2::error("Net {NET} EId {EID} MCTPEndPoint::createHostStateObserver failed.", "NET", this->getNet(), "EID", this->getEId());
        return false;
    }
    _EPHostStateObserver = new MCTPEPHostStateObserver(this, hostStateSubject);
    return true;
}

void MCTPEndPoint::notifyHostStateChange(HostState newHostState) {
    lg2::debug("Net {NET} EId {EID} MCTPEndPoint::notifyHostStateChange()", "NET", this->getNet(), "EID", this->getEId());
    //_previousHostState = _currentHostState;
    //_currentHostState = newHostState;
}

void MCTPEndPoint::notifySystemReset() {
    lg2::debug("Net {NET} EId {EID} MCTPEndPoint::notifySystemReset()", "NET", this->getNet(), "EID", this->getEId());
}

bool MCTPEndPoint::startServiceImp() {
    lg2::debug("Net {NET} EId {EID} MCTPEndPoint::startServiceImp()", "NET", this->getNet(), "EID", this->getEId());
    return true;
}

bool MCTPEndPoint::stopServiceImp() {
    lg2::debug("Net {NET} EId {EID} MCTPEndPoint::stopServiceImp()", "NET", this->getNet(), "EID", this->getEId());
    return true;
}

/* MCTPEPHostStateObserver
*
*/
MCTPEPHostStateObserver::MCTPEPHostStateObserver(MCTPEndPoint* ep, HostStateSubject* hostStateSubject)
    :_ep(ep), HostStateObserver(hostStateSubject)
{
    if (hostStateSubject)
        hostStateSubject->attach(this);
    else
        lg2::error("MCTPEPHostStateObserver::MCTPEPHostStateObserver hostStateSubject is nullptr)");
}

bool MCTPEPHostStateObserver::update() {
    if (_ep && _hostStateSubject) {
        _ep->notifyHostStateChange(_hostStateSubject->getCurrentHostState());
        return true;
    } else {
        return false;
    }
}

/* MCTPBridge
*
*/

MCTPBridge::MCTPBridge(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid)
    :MCTPEndPoint(epContext, net, eid), _RTTimer(epContext->io), _mctpNLIntf(nullptr), _epmgr(nullptr),
        _pollingHostState(HostState::off), _hostStateSubject(nullptr), _routingTableConfig(nullptr)
{

}

MCTPBridge::MCTPBridge(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid)
    :MCTPEndPoint(epContext, *netIntfInfo.net, eid), _netIntfInfo(netIntfInfo), _RTTimer(epContext->io), _mctpNLIntf(nullptr), _epmgr(nullptr),
        _pollingHostState(HostState::off), _hostStateSubject(nullptr), _routingTableConfig(nullptr)
{

}

MCTPBridge::~MCTPBridge() {
    _RTTimer.cancel();

    for ( MCTPEndPoint* ep : _endpoints) {
        ep->stopService();
        delete ep;
    }

    if (_routingTableConfig)
        delete _routingTableConfig;
}

bool MCTPBridge::createHostStateObserver(HostStateSubject* hostStateSubject) {
    if (hostStateSubject == nullptr) {
        lg2::error("Net {NET} EId {EID} MCTPEndPoint::createHostStateObserver failed.", "NET", this->getNet(), "EID", this->getEId());
        return false;
    }
    _hostStateSubject = hostStateSubject;
    return  MCTPEndPoint::createHostStateObserver(hostStateSubject);
}

void MCTPBridge::setPollingHostState(HostState newPollingHostState) {
    _pollingHostState = newPollingHostState;
}

void MCTPBridge::notifyHostStateChange(HostState newHostState) {
    MCTPEndPoint::notifyHostStateChange(newHostState);
    lg2::debug("Net {NET} EId {EID} MCTPBridge::notifyHostStateChange()", "NET", this->getNet(), "EID", this->getEId());

    if (newHostState < _pollingHostState) {
        lg2::info("Net {NET} EId {EID} MCTPBridge: Stop polling", "NET", this->getNet(), "EID", this->getEId());
        this->stopService();
    } else {
        lg2::info("Net {NET} EId {EID} MCTPBridge: Start polling", "NET", this->getNet(), "EID", this->getEId());
        this->startService();
    }
}

void MCTPBridge::notifySystemReset() {
    MCTPEndPoint::notifySystemReset();
    lg2::debug("Net {NET} EId {EID} MCTPBridge::notifySystemReset()", "NET", this->getNet(), "EID", this->getEId());
}

bool MCTPBridge::setRoutingTableConfig(const struct RoutingTableConfig& routingTableConfig) {
    if (_routingTableConfig == nullptr) {
        _routingTableConfig = new struct RoutingTableConfig;
        if (_routingTableConfig == nullptr) {
            lg2::error("Net {NET} EId {EID} MCTPBridge::setRoutingTableConfig() failed", "NET", this->getNet(), "EID", this->getEId());
            return false;
        }
    }
    lg2::debug("Net {NET} EId {EID} MCTPBridge::setRoutingTableConfig()", "NET", this->getNet(), "EID", this->getEId());
    memcpy(_routingTableConfig, &routingTableConfig, sizeof (struct RoutingTableConfig));
    return true;
}

struct RoutingTableConfig*  MCTPBridge::getRoutingTableConfig() {
    return _routingTableConfig;
}

void MCTPBridge::bindMctpNetlinkIntf(MctpNetlinkIntf* mctpNLIntf) {
    _mctpNLIntf = mctpNLIntf;
}

void MCTPBridge::bindEndpointManager(EndpointManager* epmgr) {
    _epmgr = epmgr;
}

EndpointManager* MCTPBridge::getEndpointManager() {
    return _epmgr;
}

void MCTPBridge::dumpRoutingTable(std::vector<std::uint8_t>& table) {
    struct rtEntry {
        std::uint8_t sizeEIdRange;
        std::uint8_t startEID;
        std::uint8_t entryType;
        std::uint8_t ptbId;
        std::uint8_t pmdId;
        std::uint8_t addrSize;
    };

    std::uint8_t* point = (std::uint8_t*) table.data();
    while (point < table.data() + table.size()) {
        struct rtEntry* entry = (struct rtEntry*) point;
        std::stringstream stream;
        stream << "StartEID: "  << std::setw(3) <<  (std::uint16_t) entry->startEID;
        stream << ", Binding: 0x" << std::hex << std::setfill('0') << std::setw(2) << (std::uint16_t) entry->ptbId;
        stream << ", Addr: ";

        std::uint8_t* pAddr = point + sizeof(struct rtEntry);
        for (ssize_t offset = entry->addrSize - 1; offset >=0 ; offset --) {
            stream << "0x" << std::hex << std::setfill('0') << std::setw(2) << (std::uint16_t) *(pAddr + offset);
            if (offset != 0)
                stream << ":";
        }
        stream << std::endl;
        lg2::debug("{MSG}", "MSG", stream.str());

        point += sizeof(struct rtEntry) + entry->addrSize;
    }
}

void MCTPBridge::handleRoutingTableUpdate(std::vector<std::uint8_t>& oldTable, std::vector<std::uint8_t>& newTable) {
    struct rtEntry {
        std::uint8_t sizeEIdRange;
        std::uint8_t startEID;
        std::uint8_t entryType;
        std::uint8_t ptbId;
        std::uint8_t pmdId;
        std::uint8_t addrSize;
    };
    struct RTStatus {
        std::uint8_t EId;
        std::uint8_t Binding;
        std::uint8_t addrSize;
        std::uint8_t address[5];
        int status;
    };

    std::vector<struct RTStatus> rtStatusTbl;

    std::uint8_t* point = (std::uint8_t*) oldTable.data();
    while (point < oldTable.data() + oldTable.size()) {
        struct rtEntry* entry = (struct rtEntry*) point;
        struct RTStatus rtStatus = { 0 };

        rtStatus.EId = entry->startEID;
        rtStatus.Binding = entry->ptbId;
        rtStatus.addrSize = entry->addrSize;
        std::uint8_t* pAddr = point + sizeof(struct rtEntry);
        std::memcpy(&rtStatus.address, pAddr, entry->addrSize);
        rtStatus.status = RoutingEntryStatus::Deleted;   //Inited as deleted.

        rtStatusTbl.push_back(rtStatus);
        point += sizeof(struct rtEntry) + entry->addrSize;
    }
    point = (std::uint8_t*) newTable.data();
    while (point < newTable.data() + newTable.size()) {
        struct rtEntry* entry = (struct rtEntry*) point;
        struct RTStatus rtStatus = { 0 };
        bool found = false;

        rtStatus.EId = entry->startEID;
        rtStatus.Binding = entry->ptbId;
        rtStatus.addrSize = entry->addrSize;
        std::uint8_t* pAddr = point + sizeof(struct rtEntry);
        std::memcpy(&rtStatus.address, pAddr, entry->addrSize);
        rtStatus.status = RoutingEntryStatus::Added;    //Inited as added.

        for (size_t i = 0; i < rtStatusTbl.size(); i++) {
            struct RTStatus rts = rtStatusTbl[i];
            if ((rts.EId == rtStatus.EId) && (rts.Binding == rtStatus.Binding)) {
                rts.status = RoutingEntryStatus::Unchanged;
                rtStatusTbl[i] = rts;
                found = true;
            }
        }
        if (!found) {
            rtStatusTbl.push_back(rtStatus);
        }
        point += sizeof(struct rtEntry) + entry->addrSize;
    }

    for (struct RTStatus rts : rtStatusTbl) {
        const mctp_eid_t eid = rts.EId;
        struct RoutingTableConfig* routingTable = this->getRoutingTableConfig();
        if (routingTable != nullptr) {
            if ((eid < routingTable->startEId) || (eid >= routingTable->startEId + routingTable->EIdCount)) {
                lg2::warning("Net {NET} EId {BEID} endpoint EId {EID} is not in valid EID range for the bridge.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
                continue;
            }
        }

        switch (rts.Binding) {
            case MCTPTransportBindingType::PCIE_VDM:
                {
                    const struct PCIePhyAddr pcieAddr = {rts.address[0], (rts.address[1] & 0xF8) >> 3, (rts.address[1] & 0x07)};
                    switch (rts.status) {
                        case RoutingEntryStatus::Deleted:
                            {
                                lg2::info("Net {NET} EId {BEID} removed endpoint EId {EID} from the net.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
                                this->removeEndpoint(eid, pcieAddr);
                                break;
                            }
                        case RoutingEntryStatus::Added:
                            {
                                lg2::info("Net {NET} EId {BEID} added endpoint EId {EID} to the net.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
                                this->addEndpoint(eid, pcieAddr);
                                break;
                            }
                        default:
                            lg2::warning("Net {NET} EId {BEID}: Unsupported RoutingEntryStatus {STATUS} for endpoint {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "STATUS", rts.status, "EID", eid);
                            break;
                    }
                    break;
                }
            default:
                lg2::warning("Net {NET} EId {BEID}: Unsupported binding type {TYPE} for endpoint {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "TYPE", rts.Binding, "EID", eid);
                break;
        }
    }
}

int MCTPBridge::addEndpoint(const std::uint8_t eid, const struct PCIePhyAddr addr) {
    MctpNetlinkIntf* nlIntf = this->getMctpNetlinkIntf();
    MCTPPCIeVDMBindingIntf* mctpTransBindingIntf = new MCTPPCIeVDMBindingIntf(addr);
    if ((nlIntf != nullptr) && (mctpTransBindingIntf != nullptr)){
        if ((nlIntf->delNeigh(_netIntfInfo.ifIndex, eid) >= 0) &&
            (nlIntf->addRoute(_netIntfInfo.ifIndex, eid) >= 0) &&
            (nlIntf->addNeigh(_netIntfInfo.ifIndex, eid, mctpTransBindingIntf->getlladdrBDF()) >= 0)) {
            lg2::debug("Net {NET} EId {BEID}: Added route and neigh for {EID} BDF: {BUS}, {DEV}, {FUNC}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid,
                            "BUS", addr.Bus, "DEV", addr.Dev, "FUNC", addr.Func);
        } else {
            lg2::warning("Net {NET} EId {BEID}: Failed to add route or neigh for {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
        }
    } else {
        lg2::warning("Net {NET} EId {BEID}: MctpNetlinkIntf is null.", "NET", this->getNet(), "BEID", this->getEId());
    }

    std::lock_guard<std::mutex> guard(_EPMutex);
    MCTPEndPoint* ep = new MCTPEndPoint(this->getEPContext(), this->getNet(), eid);
    ep->setMCTPTransportBindingIntf(mctpTransBindingIntf);

    if ((this->getEndpointManager() != nullptr) && (this->getEndpointManager()->bindDriver(ep) > 0)){
        lg2::debug("Net {NET} EId {BEID}: Bond driver for {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
    } else {
        lg2::warning("Net {NET} EId {BEID}: Failed to bind driver for {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
    }

    ep->startService();
    _endpoints.push_back(ep);
    return 0;
}

int MCTPBridge::removeEndpoint(const std::uint8_t eid, const struct PCIePhyAddr addr) {
    MctpNetlinkIntf* nlIntf = this->getMctpNetlinkIntf();
    if (nlIntf != nullptr) {
        if ((nlIntf->delRoute(_netIntfInfo.ifIndex, eid) >= 0) && (nlIntf->delNeigh(_netIntfInfo.ifIndex, eid) >= 0)) {
            lg2::debug("Net {NET} EId {BEID}: Removed route or neigh for {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
        } else {
            lg2::warning("Net {NET} EId {BEID}: Failed to remove route or neigh for {EID}.", "NET", this->getNet(), "BEID", this->getEId(), "EID", eid);
        }
    } else {
        lg2::warning("Net {NET} EId {BEID}: MctpNetlinkIntf is null.", "NET", this->getNet(), "BEID", this->getEId());
    }

    std::lock_guard<std::mutex> guard(_EPMutex);
    for (auto it = _endpoints.begin(); it != _endpoints.end(); ) {
        MCTPEndPoint* ep = *it;
        if ((ep->getNet() == this->getNet()) && (ep->getEId() == eid)) {
            ep->stopService();
            delete ep;
            it = _endpoints.erase(it);
        } else {
            ++it;
        }
    }
    return 0;
}

MctpNetlinkIntf* MCTPBridge::getMctpNetlinkIntf() {
    return _mctpNLIntf;
}

void MCTPBridge::updateRoutingTableWdt(boost::asio::steady_timer& timer) {
    if (this->isServiceRunning()) {
        std::uint8_t respBuffer[this->getMTU()] = { 0 };
        ssize_t respSize = 0;
        std::uint8_t entryHandle = 0x00;
        std::vector<std::uint8_t> tempTable;
        const std::uint8_t MAX_ATTEMPTS = 3;
        std::uint8_t attempts = MAX_ATTEMPTS;

        while ( this->isServiceRunning() && (attempts > 0) && (entryHandle != 0xFF)) {
            MCTPPacket<struct mctp_ctrl_handle_cmd> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
            struct mctp_ctrl_handle_cmd* req = dataPack.getReqData();
            if (req == nullptr) {
                lg2::error("Net {NET} EId {EID} Allocate memory failed.", "NET", this->getNet(), "EID", this->getEId());
                attempts = 0;
                break;
            }
            req->subCmc = MCTP_CTRL_CMD_GET_ROUTING_TBL;
            req->handle = entryHandle;

            if ((this->doSendReceive(&dataPack) == true) && (dataPack.respSize >= 4) && (dataPack.respData[2] == 0x00)) {
                tempTable.insert(tempTable.end(), dataPack.respData + 5, dataPack.respData + dataPack.respSize);
                entryHandle = dataPack.respData[3];
                attempts = MAX_ATTEMPTS;
            } else {
                lg2::warning("MCTPBridge::updateRoutingTableWdt invalid routing table?");
                attempts --;
                std::this_thread::sleep_for(std::chrono::milliseconds(100 + (500 * attempts)));
            }
        }

        if (attempts > 0) {
            // Only update routing table when attempts are normal.
            // this->dumpRoutingTable(tempTable);
            const std::uint32_t crc32 = ~nvme_mi_crc32_update(0xFFFFFFFF, tempTable.data(), tempTable.size());

            if (crc32 != this->_rtCRC32) {
                lg2::info("Net {NET} EId {EID} Routing table updated.", "NET", this->getNet(), "EID", this->getEId());
                this->handleRoutingTableUpdate(_rtRawData, tempTable);
                 _rtRawData = tempTable;
                this->_rtCRC32 = crc32;
            }
        } else {
            lg2::error("MCTPBridge::updateRoutingTableWdt failed with max attempts.");
        }

        timer.expires_after(std::chrono::seconds(5));
        timer.async_wait([&](const boost::system::error_code& error) {
            if (error != boost::asio::error::operation_aborted) {
                updateRoutingTableWdt(timer);
            }
        });
    }
}

bool MCTPBridge::startRoutingTableService() {
    lg2::debug("EndpointManager::startRoutingTableService()");
    updateRoutingTableWdt(_RTTimer);
    lg2::debug("EndpointManager::startRoutingTableService() done");
    return true;
}

bool MCTPBridge::stopRoutingTableService() {
    _RTTimer.cancel();
    return true;
}

bool MCTPBridge::startServiceImp()
{
    lg2::debug("Net {NET} EId {EID} MCTPBridge::startServiceImp()", "NET", this->getNet(), "EID", this->getEId());

    if (this->_epContext == nullptr)
        return false;

    if (this->getMctpNetlinkIntf() == nullptr)
        return false;

    MctpNetlinkIntf* nlIntf = this->getMctpNetlinkIntf();

    nlIntf->delNeigh(_netIntfInfo.ifIndex, this->getEId()); //Removed any neigh using this eid.
    nlIntf->addRoute(_netIntfInfo.ifIndex, this->getEId());

    if (this->getMCTPTransportBindingIntf() == nullptr)
        return false;

    const enum MCTPTransportBindingType bindingType = this->getMCTPTransportBindingIntf()->getBindingType();
    switch (bindingType)
    {
        case MCTPTransportBindingType::PCIE_VDM:
            {
                MCTPPCIeVDMBindingIntf* pcieIntf = dynamic_cast<MCTPPCIeVDMBindingIntf*>(this->getMCTPTransportBindingIntf());
                if (pcieIntf == nullptr) {
                    lg2::error("Unable to get physical address due to failed at casting to MCTPPCIeVDMBindingIntf.");
                    return false;
                }
                const struct lladdrBDF lladdr = pcieIntf->getlladdrBDF();
                lg2::debug("lladdr.bus: {BUS}, lladdr.devfunc: {DEVFUNC}", "BUS", lladdr.bus, "DEVFUNC", lladdr.devfunc);

                nlIntf->addNeigh(_netIntfInfo.ifIndex, this->getEId(), lladdr);
                break;
            }
        default:
            lg2::error("Unsupported MCTP Transport Binding Type: {TYPE}", "TYPE", bindingType);
            return false;
    }

    return this->startRoutingTableService();
}

bool MCTPBridge::stopServiceImp()
{
    lg2::debug("Net {NET} EId {EID} MCTPBridge::stopServiceImp()", "NET", this->getNet(), "EID", this->getEId());
    this->stopRoutingTableService();

    std::lock_guard<std::mutex> guard(_EPMutex);
    lg2::debug("Net {NET} EId {EID} MCTPBridge::stopServiceImp() to delete endpoints.", "NET", this->getNet(), "EID", this->getEId());
    for ( MCTPEndPoint* ep : _endpoints) {
        ep->stopService();
        delete ep;
    }
    _endpoints.clear();
    _rtRawData.clear();

    return true;
}

/* MCTPBusOwner
*
*/
MCTPBusOwner::MCTPBusOwner(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid)
    :MCTPBridge(epContext, net, eid)
{

}

MCTPBusOwner::MCTPBusOwner(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid)
    :MCTPBridge(epContext, netIntfInfo, eid)
{

}


/* MCTPBusBridgeAtRootComplex
*
*/
MCTPBusBridgeAtRootComplex::MCTPBusBridgeAtRootComplex(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid)
    :MCTPBridge(epContext, net, eid), _RTInitTimer(epContext->io)
{

}

MCTPBusBridgeAtRootComplex::MCTPBusBridgeAtRootComplex(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid)
    :MCTPBridge(epContext, netIntfInfo, eid), _RTInitTimer(epContext->io)
{

}

bool MCTPBusBridgeAtRootComplex::startServiceImp()
{
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::startServiceImp()", "NET", this->getNet(), "EID", this->getEId());

    if (this->_epContext == nullptr)
        return false;

    if (this->getMctpNetlinkIntf() == nullptr)
        return false;

    MctpNetlinkIntf* nlIntf = this->getMctpNetlinkIntf();

    nlIntf->delNeigh(_netIntfInfo.ifIndex, this->getEId()); //Removed any neigh using this eid.
    nlIntf->addRoute(_netIntfInfo.ifIndex, this->getEId());

    if (this->getMCTPTransportBindingIntf() == nullptr)
        return false;

    const enum MCTPTransportBindingType bindingType = this->getMCTPTransportBindingIntf()->getBindingType();
    switch (bindingType)
    {
        case MCTPTransportBindingType::PCIE_VDM:
            {
                MCTPPCIeVDMBindingIntf* pcieIntf = dynamic_cast<MCTPPCIeVDMBindingIntf*>(this->getMCTPTransportBindingIntf());
                if (pcieIntf == nullptr) {
                    lg2::error("Unable to get physical address due to failed at casting to MCTPPCIeVDMBindingIntf.");
                    return false;
                }
                const struct lladdrBDF lladdr = pcieIntf->getlladdrBDF();
                lg2::debug("lladdr.bus: {BUS}, lladdr.devfunc: {DEVFUNC}", "BUS", lladdr.bus, "DEVFUNC", lladdr.devfunc);

                nlIntf->addNeigh(_netIntfInfo.ifIndex, this->getEId(), lladdr);
                break;
            }
        default:
            lg2::error("Unsupported MCTP Transport Binding Type: {TYPE}", "TYPE", bindingType);
            return false;
    }

    //TODO: Init might failed. Need more robust implementation.
    this->RTInitWdt(_RTInitTimer);
    return true;
}

bool MCTPBusBridgeAtRootComplex::stopServiceImp()
{
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::stopServiceImp()", "NET", this->getNet(), "EID", this->getEId());
    _RTInitTimer.cancel();
    MCTPBridge::stopServiceImp();
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::stopServiceImp() done", "NET", this->getNet(), "EID", this->getEId());
    return true;
}

void MCTPBusBridgeAtRootComplex::RTInitWdt(boost::asio::steady_timer& timer) {
    if (this->initBusBridge()) {
        this->startRoutingTableService();
    } else {
        timer.expires_after(std::chrono::seconds(3));
        timer.async_wait([&](const boost::system::error_code& error) {
            if (error != boost::asio::error::operation_aborted) {
                this->RTInitWdt(timer);
            }
        });
    }
}

bool MCTPBusBridgeAtRootComplex::initBusBridge() {
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::initBusBridge()", "NET", this->getNet(), "EID", this->getEId());
    if (this->sendPrepareEndpointDiscovery() &&
            this->sendEndpointDiscovery() &&
            this->sendSetEndpointID() &&
            this->sendAllocateEndpointIDs()) {
        return true;
    } else {
        return false;
    }
}

bool MCTPBusBridgeAtRootComplex::sendPrepareEndpointDiscovery() {
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendPrepareEndpointDiscovery()", "NET", this->getNet(), "EID", this->getEId());
    MCTPPacket<struct mctp_ctrl_generic_cmd> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
    dataPack.verboseLog = true;
    struct mctp_ctrl_generic_cmd* req = dataPack.getReqData();
    req->subCmc = 0x0B;
    if (this->doSendReceive(&dataPack))
        return true;

    lg2::error("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendPrepareEndpointDiscovery() failed", "NET", this->getNet(), "EID", this->getEId());
    return false;
}

bool MCTPBusBridgeAtRootComplex::sendEndpointDiscovery() {
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendEndpointDiscovery()", "NET", this->getNet(), "EID", this->getEId());
    MCTPPacket<struct mctp_ctrl_generic_cmd> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
    dataPack.verboseLog = true;
    struct mctp_ctrl_generic_cmd* req = dataPack.getReqData();
    req->subCmc = 0x0C;
    if (this->doSendReceive(&dataPack))
        return true;
    lg2::error("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendEndpointDiscovery() failed", "NET", this->getNet(), "EID", this->getEId());
    return false;
}

bool MCTPBusBridgeAtRootComplex::sendSetEndpointID() {
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendSetEndpointID()", "NET", this->getNet(), "EID", this->getEId());
    MCTPPacket<struct mctp_ctrl_set_endpoint_id> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
    dataPack.verboseLog = true;
    struct mctp_ctrl_set_endpoint_id* req = dataPack.getReqData();
    req->subCmc = 0x01;
    req->op = 0x00;
    req->eid = this->getEId();
    if (this->doSendReceive(&dataPack))
        return true;
    lg2::error("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendSetEndpointID() failed", "NET", this->getNet(), "EID", this->getEId());
    return false;
}

bool MCTPBusBridgeAtRootComplex::sendAllocateEndpointIDs() {
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendAllocateEndpointIDs()", "NET", this->getNet(), "EID", this->getEId());
    struct RoutingTableConfig* routingTable = this->getRoutingTableConfig();
    if (routingTable == nullptr) {
        lg2::warning("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendAllocateEndpointIDs() Unable to get RoutingTableConfig.", "NET", this->getNet(), "EID", this->getEId());
        return true;  // Intentionally return true so that it won't keeping retry for initialization.
    }
    MCTPPacket<struct mctp_ctrl_allocate_eids> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
    dataPack.verboseLog = true;
    struct mctp_ctrl_allocate_eids* req = dataPack.getReqData();
    req->subCmc = 0x08;
    req->op = 0x01;
    req->eidCount = routingTable->EIdCount;
    req->startEId = routingTable->startEId;
    lg2::debug("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendAllocateEndpointIDs() StartEID: {STARTEID}, Count: {COUNT}", "NET", this->getNet(), "EID", this->getEId(),
            "STARTEID", req->startEId, "COUNT", req->eidCount);
    if (this->doSendReceive(&dataPack))
        return true;
    lg2::error("Net {NET} EId {EID} MCTPBusBridgeAtRootComplex::sendAllocateEndpointIDs() failed", "NET", this->getNet(), "EID", this->getEId());
    return false;
}

/* EndpointManager
*
*/
EndpointManager::EndpointManager(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr)
    :_Net(net), _BO_EID(eid), _BO_Addr(addr), _SocketDesc(-1), _hasInited(false), _instanceId(0), _MTU(68)
{
    _epContext.socketDesc = socket(AF_MCTP, SOCK_DGRAM, 0);
    _mctpNetlinkIntf = MctpNetlinkIntf::getMctpNetlinkIntf(&_epContext);
    _hostStateSubject = new Ast2600PEHRC4HostStateSubject();
}

EndpointManager::~EndpointManager() {
    if (_hasInited > 0) {
        close(_SocketDesc);
    }

    if (_hostStateSubject != nullptr) {
        delete _hostStateSubject;
    }

    for ( MCTPBusOwner* bo : _BusOwners) {
        delete bo;
    }

    for ( MCTPBridge* bb : _BusBridges) {
        delete bb;
    }

    for ( MCTPEndPoint* ep : _endpoints) {
        ep->stopService();
        delete ep;
    }

    if (_mctpNetlinkIntf != nullptr)
        delete _mctpNetlinkIntf;
}

int EndpointManager::getSocket()
{
    if (_hasInited)
        return _SocketDesc;

    _SocketDesc = socket(AF_MCTP, SOCK_DGRAM, 0); /* create the MCTP socket */
    if (_SocketDesc > 0) {
        struct timeval timeout;
        timeout.tv_sec = 3; // 3 seconds
        timeout.tv_usec = 0;
        if (setsockopt(_SocketDesc, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
            std::cerr << "Set timeout for Receive failed." << std::endl;
        if (setsockopt(_SocketDesc, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
            std::cerr << "Set timeout for Send failed." << std::endl;

        _hasInited = true;
    }
    return _SocketDesc;
}

std::uint8_t EndpointManager::getInstanceId()
{
    if (_instanceId++ > 0x1F)
        _instanceId = 0;
    return _instanceId;
}

std::uint16_t EndpointManager::getMTU()
{
    return _MTU;
}

// Deprecated.
void EndpointManager::loadDrivers() {
    mctpDrvMgr.loadDriver("/tmp/libmctp-nvme-mi-generic-driver.so");
    mctpDrvMgr.loadDriver("/tmp/libmctp-spdm-generic-driver.so");
    mctpDrvMgr.loadDriver("/tmp/libbrcm-vdm-storelib7-driver.so");
}

void EndpointManager::loadDrivers(const nlohmann::json& jsonObj) {
    Json drivers = jsonObj.at("Drivers");
    if (!drivers.is_array())
        lg2::error("Invalid statement in json config: Drivers property shall be Json Array.");

    for (Json::iterator it = drivers.begin(); it != drivers.end(); ++it) {
        if ((*it).is_string())
            mctpDrvMgr.loadDriver(*it);
        else
            lg2::warning("Invalid statement in json config: {WHAT}", "WHAT", *it);
    }
}

/*
To be deprecated as the endpoint will be launch by bridge or bus owner endpoint.
void EndpointManager::bindDrivers() {
    for (MCTPEndPointBase* ep : _endpoints) {
        lg2::info("EndpointManager::bindDrivers: Binding driver for Net {NET} EId {EID}", "NET", ep->getNet(), "EID", ep->getEId());
        mctpDrvMgr.bindDriver(ep);
    }
}
*/

/*
// To be deprecated as the endpoint will be launch by bridge or bus owner endpoint.
void EndpointManager::startEPService() {
    std::cout << "EndpointManager::startEPService" << std::endl;
    lg2::info("EndpointManager::bindDrivers: Binding driver for Net {NET} EId {EID}", "NET", ep->getNet(), "EID", ep->getEId());
    for (MCTPEndPointBase* ep : _endpoints) {
        std::cout << "ep->startService Net: " << (uint16_t) ep->getNet() << ", EID: " << (uint16_t) ep->getEId()<< std::endl;
        ep->startService();
    }
}
*/

struct EPContext* EndpointManager::getEPContext()
{
    return &_epContext;
}

int EndpointManager::addEndpoint(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr) {
    std::lock_guard<std::mutex> guard(_EPMutex);
    MCTPPCIeVDMBindingIntf* mctpTransBindingIntf = new MCTPPCIeVDMBindingIntf(addr);
    MCTPEndPoint* ep = new MCTPEndPoint(this->getEPContext(), net, eid);
    ep->setMCTPTransportBindingIntf(mctpTransBindingIntf);
    mctpDrvMgr.bindDriver(ep);
    ep->startService();
    _endpoints.push_back(ep);
    return 0;
}

int EndpointManager::removeEndpoint(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr) {
    std::lock_guard<std::mutex> guard(_EPMutex);
    for (auto it = _endpoints.begin(); it != _endpoints.end(); ) {
        MCTPEndPoint* ep = *it;
        if ((ep->getNet() == net) && (ep->getEId() == eid)) {
            ep->stopService();
            delete ep;
            it = _endpoints.erase(it);
        } else {
            ++it;
        }
    }
    return 0;
}

bool EndpointManager::startService()
{
    MCTPEndPoint* ep1 = nullptr;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        work_guard = boost::asio::make_work_guard(this->getEPContext()->io);

    this->_mctpNetlinkIntf->startService();
    lg2::debug("EndpointManager::startService() cp1");
    if (this->_hostStateSubject != nullptr)
        this->_hostStateSubject->startService(&this->getEPContext()->io);

    lg2::debug("EndpointManager::startService() cp2");
    this->getEPContext()->io.run();
    lg2::debug("EndpointManager::startService() cp3");
    /*
    while (true) {
        sleep(1);
    };
    */
    return true;
}

void EndpointManager::attachPCIeBusOwnerEndpoint(const std::string moduleName, const std::string intfName, const mctp_eid_t eid, const struct PCIePhyAddr phyAddr) {
    int ifindex = 0;
    if (_mctpNetlinkIntf == nullptr) {
        lg2::error("Needs to bind _mctpNetlinkIntf");
        return;
    }

    struct NetIntfInfo intfInfo;
    if (_mctpNetlinkIntf->getLink(intfName, intfInfo) < 0) {
        lg2::error("Unable to find {NAME}", "NAME", intfName);
        return;
    }

    if (!intfInfo.net) {
        lg2::error("Unable to get Net from MCTP Network Interface {NAME}", "NAME", intfName);
        return;
    }

    /*
    if (intfInfo.mtu) {
        lg2::debug("MTU for {NAME} is {MTU}.", "NAME", intfName, "MTU", *intfInfo.mtu);
    }
    */

    lg2::info("Attach PCIe Bus Owner Endpoint: {NAME} Net: {NET} MTU: {MTU}", "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
    MCTPPCIeVDMBindingIntf* mctpTransBindingIntf = new MCTPPCIeVDMBindingIntf(phyAddr);
    MCTPBusOwner* bo = nullptr;

    if (moduleName == "MCTPBusOwnerAtRootComplex") {
        lg2::info("Attach PCIe Bus Owner Endpoint with {MODULE}: {NAME} Net: {NET} MTU: {MTU}", "MODULE", moduleName, "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        bo = new MCTPBusOwner(this->getEPContext(), intfInfo, eid);
    } else {
        lg2::info("Attach PCIe Bus Owner Endpoint with default MCTPBusOwnerAtRootComplex module: {NAME} Net: {NET} MTU: {MTU}", "MODULE", moduleName, "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        bo = new MCTPBusOwner(this->getEPContext(), intfInfo, eid);
    }

    if (bo != nullptr) {
        bo->setMCTPTransportBindingIntf(mctpTransBindingIntf);
        bo->bindMctpNetlinkIntf(_mctpNetlinkIntf);
        bo->bindEndpointManager(this);
        bo->setPollingHostState(HostState::postComplete);
        bo->createHostStateObserver(_hostStateSubject);
        _BusOwners.push_back(bo);
    } else {
        lg2::error("Unable to attach PCIe Bus Owner Endpoint: {NAME} Net: {NET} MTU: {MTU}", "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        delete mctpTransBindingIntf;
    }
}

void EndpointManager::attachPCIeBusBridgeEndpoint(const std::string moduleName, const std::string intfName, const mctp_eid_t eid, const struct PCIePhyAddr phyAddr, const struct RoutingTableConfig routingTableConfig) {
    int ifindex = 0;
    if (_mctpNetlinkIntf == nullptr) {
        lg2::error("Needs to bind _mctpNetlinkIntf");
        return;
    }

    struct NetIntfInfo intfInfo;
    if (_mctpNetlinkIntf->getLink(intfName, intfInfo) < 0) {
        lg2::error("Unable to find {NAME}", "NAME", intfName);
        return;
    }

    if (!intfInfo.net) {
        lg2::error("Unable to get Net from MCTP Network Interface {NAME}", "NAME", intfName);
        return;
    }

    MCTPPCIeVDMBindingIntf* mctpTransBindingIntf = new MCTPPCIeVDMBindingIntf(phyAddr);
    MCTPBridge* bb = nullptr;

    if (moduleName == "MCTPBusBridgeAtRootComplex") {
        lg2::info("Attach PCIe Bus Bridge Endpoint with {MODULE} module: {NAME} Net: {NET} MTU: {MTU}", "MODULE", moduleName, "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        bb = new MCTPBusBridgeAtRootComplex(this->getEPContext(), intfInfo, eid);
    } else {
        // Give a default config
        lg2::info("Attach PCIe Bus Bridge Endpoint with default MCTPBusBridgeAtRootComplex module: {NAME} Net: {NET} MTU: {MTU}", "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        bb = new MCTPBusBridgeAtRootComplex(this->getEPContext(), intfInfo, eid);
    }

    if (bb != nullptr) {
        bb->setMCTPTransportBindingIntf(mctpTransBindingIntf);
        bb->bindMctpNetlinkIntf(_mctpNetlinkIntf);
        bb->bindEndpointManager(this);
        bb->setPollingHostState(HostState::postComplete);
        bb->createHostStateObserver(_hostStateSubject);
        bb->setRoutingTableConfig(routingTableConfig);
        _BusBridges.push_back(bb);
    } else {
        lg2::error("Unable to attach PCIe Bus Bridge Endpoint: {NAME} Net: {NET} MTU: {MTU}", "NAME", intfName, "NET", *intfInfo.net, "MTU", *intfInfo.mtu);
        delete mctpTransBindingIntf;
    }
}

void EndpointManager::attachBusOwnerEndpoint(const nlohmann::json& jsonObj) {
    if (!jsonObj.contains("BusOwners"))
        return;

    Json jBOs = jsonObj.at("BusOwners");
    if (!jBOs.is_array()) {
        lg2::error("BusOwners shall be json array.");
        return;
    }

    for (Json::iterator it = jBOs.begin(); it != jBOs.end(); ++it) {
        Json jBODef = *it;
        if (!jBODef.is_object()) {
            lg2::warning("Invalid statement in json config: {WHAT}", "WHAT", *it);
            continue;
        }
        if (!jBODef.contains("MCTPPhyTransBindId") || !jBODef.contains("IntfName") || !jBODef.contains("EId")) {
            lg2::error("Lost mandatory property: MCTPPhyTransBindId, IntfName and Eid");
            continue;
        }
        lg2::debug("Valid BusOwner definition");

        int jMCTPPhyTransBindId = jBODef.at("MCTPPhyTransBindId");
        std::string jIntfName = jBODef.at("IntfName");
        std::string jModuleName = "Default";

        if (jBODef.contains("Module")) {
            jModuleName = jBODef.at("Module");
        }
        int jEId = jBODef.at("EId");
        switch (jMCTPPhyTransBindId) {
            case 2:
                {
                    lg2::debug("BusOwner definition for PCIe VDM");
                    if (!jBODef.contains("PciVDMAddr")) {
                        lg2::error("PciVDMAddr object is mandatory for PCIe VDM BO.");
                        continue;
                    }
                    Json jPciVDMAddr = jBODef.at("PciVDMAddr");
                    if (!jPciVDMAddr.contains("Bus") || !jPciVDMAddr.contains("Dev") || !jPciVDMAddr.contains("Func")) {
                        lg2::error("PciVDMAddr shall contain Bus, Dev and Func attribute.");
                        continue;
                    }
                    struct PCIePhyAddr pcieAddr = { jPciVDMAddr.at("Bus"), jPciVDMAddr.at("Dev"), jPciVDMAddr.at("Func") };
                    this->attachPCIeBusOwnerEndpoint(jModuleName, jIntfName, jEId, pcieAddr);
                    break;
                }
            default:
                lg2::warning("Unsupported BusOwner definition");
                break;
        }
    }
}

int EndpointManager::bindDriver(MCTPEndPoint* ep) {
    return mctpDrvMgr.bindDriver(ep);
}

void EndpointManager::attachBusBridgeEndpoint(const nlohmann::json& jsonObj) {
    if (!jsonObj.contains("BusBridges"))
        return;

    Json jBBs = jsonObj.at("BusBridges");
    if (!jBBs.is_array()) {
        lg2::error("BusBridges shall be json array.");
        return;
    }

    for (Json::iterator it = jBBs.begin(); it != jBBs.end(); ++it) {
        Json jBBDef = *it;
        if (!jBBDef.is_object()) {
            lg2::warning("Invalid statement in json config: {WHAT}", "WHAT", *it);
            continue;
        }
        if (!jBBDef.contains("MCTPPhyTransBindId") || !jBBDef.contains("IntfName") ||
                !jBBDef.contains("EId") || !jBBDef.contains("RoutingTableConfig")) {
            lg2::error("May lost any of mandatory property: MCTPPhyTransBindId, IntfName, Eid,  RoutingTableConfig");
            continue;
        }
        lg2::debug("Valid BusOwner definition");

        int jMCTPPhyTransBindId = jBBDef.at("MCTPPhyTransBindId");
        std::string jIntfName = jBBDef.at("IntfName");
        std::string jModuleName = "Default";

        Json jRoutingTableConfig = jBBDef.at("RoutingTableConfig");
        if (!jRoutingTableConfig.is_object() || !jRoutingTableConfig.contains("StartEId") || !jRoutingTableConfig.contains("EIdCount")) {
            lg2::error("May lost any of mandatory property in RoutingTableConfig: StartEId, EIdCount");
            continue;
        }
        struct RoutingTableConfig routingTableConfig;
        routingTableConfig.startEId = jRoutingTableConfig.at("StartEId");
        routingTableConfig.EIdCount = jRoutingTableConfig.at("EIdCount");

        if (jBBDef.contains("Module")) {
            jModuleName = jBBDef.at("Module");
        }

        int jEId = jBBDef.at("EId");
        switch (jMCTPPhyTransBindId) {
            case 2:
                {
                    lg2::debug("BusBridge definition for PCIe VDM");
                    if (!jBBDef.contains("PciVDMAddr")) {
                        lg2::error("PciVDMAddr object is mandatory for PCIe VDM BO.");
                        continue;
                    }
                    Json jPciVDMAddr = jBBDef.at("PciVDMAddr");
                    if (!jPciVDMAddr.contains("Bus") || !jPciVDMAddr.contains("Dev") || !jPciVDMAddr.contains("Func")) {
                        lg2::error("PciVDMAddr shall contain Bus, Dev and Func attribute.");
                        continue;
                    }
                    struct PCIePhyAddr pcieAddr = { jPciVDMAddr.at("Bus"), jPciVDMAddr.at("Dev"), jPciVDMAddr.at("Func") };
                    this->attachPCIeBusBridgeEndpoint(jModuleName, jIntfName, jEId, pcieAddr, routingTableConfig);
                    break;
                }
            default:
                lg2::warning("Unsupported BusBridge definition");
                break;
        }
    }
}

int EndpointManager::loadConfig(std::string fn) {
    if (fs::exists(fn.c_str())) {
        std::ifstream ifs;
        try {
            ifs.open(fn.c_str());
            try {
                Json jsonConfig = Json::parse(ifs);
                this->loadDrivers(jsonConfig);
                this->attachBusOwnerEndpoint(jsonConfig);
                this->attachBusBridgeEndpoint(jsonConfig);
                ifs.close();
                return 0;
            } catch (Json::parse_error& ex) {
                lg2::error("parse error at byte {BYTE} ", "BYTE", ex.byte);
                ifs.close();
                return -1;
            }
        } catch (const std::exception& ex) {
            lg2::error("Exception: {WHAT}", "WHAT", ex.what());
            return -1;
        }
    } else {
        lg2::error("Can't find MMPVCD from config. {PATH}", "PATH", fn);
        return false;
    }
    return 0;
}
