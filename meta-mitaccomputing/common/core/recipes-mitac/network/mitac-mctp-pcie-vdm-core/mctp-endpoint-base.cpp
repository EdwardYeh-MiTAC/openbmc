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
#include <mctp-endpoint-base.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <phosphor-logging/lg2.hpp>
#include <mutex>

std::mutex coreMutex;

namespace com::mitac_computing::mctp
{
uint32_t nvme_mi_crc32_update(uint32_t crc, void *data, size_t len)
{
    int i;

    while (len--) {
        crc ^= *(unsigned char *)(data++);
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0x82F63B78 : 0);
    }
    return crc;
}

bool MCTPSocketUtils::doSendReceive(const int socket, const struct MCTPAddrInfo destAddr, MCTPPacketWrap* dataPack)
{
    ssize_t rc, recvlen, ctr;
    socklen_t addrlen;
    std::uint8_t *recv_buffer;
    struct sockaddr_mctp addr;
    struct timeval tv;
    std::stringstream stream;
    tv.tv_sec = 2 ;
    tv.tv_usec = 0 ;

    std::lock_guard<std::mutex> lock(coreMutex);

    try {
        // Invliad request buffer
        if (dataPack == nullptr || dataPack->reqData == nullptr) {
            throw std::runtime_error("Invalid dataPack.");
        }
        // Invalid request length
        if (dataPack->reqSize == 0) {
            throw std::runtime_error("Invlid reqSize in dataPack.");
        }

        if (socket < 0) {
            std::cerr << "Invlid socket descriptor" << std::endl;
            throw std::runtime_error("Invlid socket descriptor");
        }

        if (dataPack->isMIC) {
            uint32_t crc_payload = ~nvme_mi_crc32_update(0xFFFFFFFF, dataPack->reqData, dataPack->reqSize - 4);
            dataPack->reqData[dataPack->reqSize - 4] = crc_payload & 0xff;
            dataPack->reqData[dataPack->reqSize - 3] = (crc_payload >>  8) & 0xff;
            dataPack->reqData[dataPack->reqSize - 2] = (crc_payload >> 16) & 0xff;
            dataPack->reqData[dataPack->reqSize - 1] = (crc_payload >> 24) & 0xff;
        }

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
            lg2::error("Set timeout for Receive failed.");
        if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
            lg2::error("Set timeout for Send failed.");
    } catch (const std::runtime_error& e) {
        lg2::error ("Exception: {WHAT}\n", "WHAT", e.what());
        return false;
    }

    // Below will dump request or response message from exception.
    try {
        if (dataPack->verboseLog) {
            stream << "MCTPSocketUtils::doSendReceive to Net " << destAddr.net <<
                                                       " EId " << destAddr.eid <<
                                                       " Type " << "0x" << std::hex << std::setfill('0') << std::setw(2) << dataPack->msgType << std::endl;
            stream << "  Req: " << std::endl << "    ";
            for (ssize_t offset = 0; offset < dataPack->reqSize; offset ++) {
                if ((offset != 0) && (offset % 16 == 0))
                    stream << std::endl << "    ";
                stream << "0x" << std::hex << std::setfill('0') << std::setw(2) << dataPack->reqData[offset] << " ";
            }
            stream << std::endl;
        }

        memset(&addr, 0, sizeof(addr));
        addr.smctp_tag = MCTP_TAG_OWNER;
        addr.smctp_family = AF_MCTP;
        addr.smctp_network = destAddr.net;
        addr.smctp_type = dataPack->msgType | (dataPack->isMIC ? 0x80 : 0x00);
        addr.smctp_addr.s_addr = destAddr.eid;

        rc = sendto(socket, dataPack->reqData + 1, dataPack->reqSize - 1, 0,
                    (struct sockaddr *)&addr, sizeof(addr));
        if (rc < 0)
            throw std::runtime_error("Failed to transfer message.");
        if (rc != dataPack->reqSize - 1)
            throw std::runtime_error("Invalid transfer size");

        dataPack->respSize = recvfrom(socket, NULL, 0, MSG_TRUNC | MSG_PEEK,
                   (struct sockaddr *)&addr, &addrlen);

        if (dataPack->respSize < 0)
            throw std::runtime_error("Failed to receive message");

        dataPack->respData = (std::uint8_t*) malloc(dataPack->respSize);

        rc = recvfrom(socket, (void*) dataPack->respData, dataPack->respSize, MSG_TRUNC,
                  (struct sockaddr *)&addr, &addrlen);
        if (rc < 0)
            throw std::runtime_error("Failed to receive message");

        if (dataPack->respSize != rc)
            throw std::runtime_error("Invalid bytes received");

        if (dataPack->verboseLog) {
            stream << "  Resp: " << std::endl << "    ";
            for (ssize_t offset = 0; offset < dataPack->respSize; offset ++) {
                if ((offset != 0) && (offset % 16 == 0))
                    stream << std::endl << "    ";
                stream << "0x" << std::hex << std::setfill('0') << std::setw(2) << dataPack->respData[offset] << " ";
            }
            stream << std::endl;
            lg2::debug("{MSG}", "MSG", stream.str());
        }
        return true;
    } catch (const std::runtime_error& e) {
        lg2::error ("Exception: {WHAT}\n{MSG}", "WHAT", e.what(), "MSG", stream.str());
        dataPack->respSize = 0;
        return false;
    }
}


MCTPEndPointBase::MCTPEndPointBase(struct EPContext* epContext, const std::uint8_t net, const mctp_eid_t eid)
            :_epContext(epContext), _Net(net), _EId(eid), _MTU(68), _mctpTransBindingIntf(nullptr), _instanceId(0), _DHTimer(epContext->io), _serviceRunning(false)
{

}

MCTPEndPointBase::~MCTPEndPointBase() {
    lg2::debug("Destruct MCTPEndPoint at Net {NET} EId {EID}", "NET", this->getNet(), "EID", this->getEId());
    std::lock_guard<std::mutex> guard(_MCTPDriverHandleMutex);
    _DHTimer.cancel();
    if (_mctpTransBindingIntf != nullptr) {
        lg2::debug("Delete _mctpTransBindingIntf");
        delete _mctpTransBindingIntf;
    }

    for (auto it = _mctpDriverHandles.begin(); it != _mctpDriverHandles.end(); ++it) {
        MCTPDriverHandle* handler = (MCTPDriverHandle*) it->second;
        lg2::debug("Delete MCTPDriverHandle from {NAME}.", "NAME", it->first);
        delete handler;
    }
    _mctpDriverHandles.clear(); //May not necessary
    lg2::debug("Destruct MCTPEndPoint successfully.");
}

bool MCTPEndPointBase::doSendReceive(MCTPPacketWrap* dataPack) const {
    if (dataPack == nullptr)
        return false;

    MCTPTransportBindingIntf* bindingIntf = this->getMCTPTransportBindingIntf();
    if (bindingIntf != nullptr)
    {
        struct MCTPAddrInfo destAddr;
        destAddr.net = this->getNet(); 
        destAddr.eid = this->getEId();
        return bindingIntf->doSendReceive(this->getSocket(), destAddr, dataPack);
    }
    return false;
}

struct EPContext* MCTPEndPointBase::getEPContext() {
    return _epContext;
}

void MCTPEndPointBase::dumpMsgTypes() {
    std::stringstream stream;
    stream << "Net " << (std::uint16_t) this->getNet() << " EID " << (std::uint16_t) this->getEId() << " supported MCTP Msg Types: ";
    for (std::uint8_t msgType :  _msgTypes) {
        stream << std::setfill('0');
        stream << "0x" << std::setw(2) << std::hex << (std::uint16_t) msgType << ", ";
    }
    stream << std::endl;
    lg2::info("{MSG}", "MSG", stream.str());
}

void MCTPEndPointBase::dumpPciVdmMsgTypes() {
    std::stringstream stream;
    stream << "Net " << (std::uint16_t) this->getNet() << " EID " << (std::uint16_t) this->getEId() << " supported PCI VDM Msg Types: ";
    if (_PciVdmMsgTypes.size() == 0) {
        stream << "None." << std::endl;
    } else {
        stream << std::endl;
        for (struct PciVdmMsgTypeId msgType :  _PciVdmMsgTypes) {
            stream << std::setfill('0');
            stream << "    Vendor ID: 0x" << std::setw(4) << std::hex << msgType.PciVendorId;
            stream << ", ExtID: 0x" << std::setw(4) << std::hex << msgType.VendorIdExt << std::endl;
        }
    }
    lg2::info("{MSG}", "MSG", stream.str());
}

void MCTPEndPointBase::dumpIanaVdmMsgTypes() {

}

bool MCTPEndPointBase::getMessageSupport() {
    lg2::debug("Net {NET} EId {EID} MCTPEndPointBase::getMessageSupport()", "NET", this->getNet(), "EID", this->getEId());
    MCTPPacket<struct mctp_ctrl_generic_cmd> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
    struct mctp_ctrl_generic_cmd* req = dataPack.getReqData();
    if (req == nullptr) {
        std::cerr << "Allocate memory failed." << std::endl;
        return false;
    }
    req->subCmc = MCTP_CTRL_CMD_SET_GET_MSG_TYPE;

    if (!this->doSendReceive(&dataPack))
        return false;

    if (dataPack.respSize <= 0x04)
        return false;

    if ((dataPack.respData[1] != MCTP_CTRL_CMD_SET_GET_MSG_TYPE) || (dataPack.respData[2] != 0x00))
        return false;

    const std::uint8_t OffsetMsgTypeCount = 3;
    const ssize_t msgCount = dataPack.respData[OffsetMsgTypeCount];
    for (ssize_t offset = 1; offset <= msgCount; offset ++) {
        _msgTypes.push_back(dataPack.respData[OffsetMsgTypeCount + offset]);
    }
    return true;    
}

bool MCTPEndPointBase::getVendorDefinedMessageSupport() {
    lg2::debug("Net {NET} EId {EID} MCTPEndPointBase::getVendorDefinedMessageSupport()", "NET", this->getNet(), "EID", this->getEId());
    std::uint8_t vendorIDSetSelector = 0x00;

    while (vendorIDSetSelector != 0xFF)
    {
        MCTPPacket<struct mctp_ctrl_handle_cmd> dataPack(false, MCTP_MSG_TYPE_CONTROL, this->getInstanceId());
        struct mctp_ctrl_handle_cmd* req = dataPack.getReqData();
        if (req == nullptr) {
            std::cerr << "Allocate memory failed." << std::endl;
            return false;
        }
        req->subCmc = MCTP_CTRL_CMD_SET_GET_VDM_TYPE;
        req->handle = vendorIDSetSelector;

        if (!this->doSendReceive(&dataPack))
            return false;

        if (dataPack.respSize <= 0x04)
            return false;

        if ((dataPack.respData[1] != MCTP_CTRL_CMD_SET_GET_VDM_TYPE) || (dataPack.respData[2] != 0x00))
            return false;

        const std::uint8_t OffsetVendorIDSetSelector = 3;
        vendorIDSetSelector = dataPack.respData[OffsetVendorIDSetSelector];
        const std::uint8_t OffsetVendorIDFormat = 4;
        const std::uint8_t vendorIDFormat = dataPack.respData[OffsetVendorIDFormat];

        switch (vendorIDFormat) {
            case 0x00:
            {
                struct PciVdmMsgTypeId typeId;
                typeId.PciVendorId = (dataPack.respData[OffsetVendorIDFormat + 1] << 8) + dataPack.respData[OffsetVendorIDFormat + 2];
                typeId.VendorIdExt = (dataPack.respData[OffsetVendorIDFormat + 3] << 8) + dataPack.respData[OffsetVendorIDFormat + 4];
                _PciVdmMsgTypes.push_back(typeId);
                break;
            }
            case 0x01:
            {
                struct IanaVdmMsgTypeId typeId;
                typeId.Iana = (dataPack.respData[OffsetVendorIDFormat + 1] << 24) +
                              (dataPack.respData[OffsetVendorIDFormat + 2] << 16) +
                              (dataPack.respData[OffsetVendorIDFormat + 3] << 8) +
                              dataPack.respData[OffsetVendorIDFormat + 4];
                typeId.IanaExt = (dataPack.respData[OffsetVendorIDFormat + 5] << 8) + dataPack.respData[OffsetVendorIDFormat + 6];
                _IanaVdmMsgTypes.push_back(typeId);
                break;
            }
            default:
                return false;
        }
    }
    return true;
}


int MCTPEndPointBase::getSocket() const {
    if (_epContext == nullptr)
        return -1;
    return _epContext->socketDesc;
}

std::uint8_t MCTPEndPointBase::getNet() const {
    return _Net;
}

std::uint8_t MCTPEndPointBase::getEId() const {
    return _EId;
}

std::uint16_t MCTPEndPointBase::getMTU() const {
    return _MTU;
}

std::vector<struct PciVdmMsgTypeId> MCTPEndPointBase::getPciVdmMsgTypes() const {
    return _PciVdmMsgTypes;
}


void MCTPEndPointBase::runDriverTask(boost::asio::steady_timer& timer) {
    lg2::debug("Net {NET} EId {EID} MCTPEndPointBase::runDriverTask", "NET", this->getNet(), "EID", this->getEId());
    std::lock_guard<std::mutex> guard(_MCTPDriverHandleMutex);
    for (auto it = _mctpDriverHandles.begin(); it != _mctpDriverHandles.end(); ++it) {
        MCTPDriverHandle* handler = (MCTPDriverHandle*) it->second;
        lg2::debug("Net {NET} EId {EID} Task: {TASK}", "NET", this->getNet(), "EID", this->getEId(), "TASK", it->first);
        handler->runTask();
    }
    // To avoid timer rearm again after previous time consuming access.
    if (this->isServiceRunning())
        timer.expires_after(std::chrono::seconds(5));

    timer.async_wait([&](const boost::system::error_code& error) {
        if (error != boost::asio::error::operation_aborted) {
            runDriverTask(timer);
        }
    });
}

bool MCTPEndPointBase::startService() {
    lg2::info("Net {NET} EId {EID} MCTPEndPointBase::startService()", "NET", this->getNet(), "EID", this->getEId());
    _serviceRunning = true;
    runDriverTask(_DHTimer);
    return startServiceImp();
}

bool MCTPEndPointBase::stopService() {
    lg2::info("Net {NET} EId {EID} MCTPEndPointBase::stopService()", "NET", this->getNet(), "EID", this->getEId());
    _serviceRunning = false;
    _DHTimer.cancel();
    return stopServiceImp();
}

bool MCTPEndPointBase::isServiceRunning() {
    return _serviceRunning;
}

bool MCTPEndPointBase::installDriverHandle(const MCTPDriverHandle* handle)
{
    if (handle == nullptr)
        return false;
    
    const std::string driverUUID = handle->getDriverUUID();
    if (_mctpDriverHandles.find(driverUUID) == _mctpDriverHandles.end()) {  
        _mctpDriverHandles.insert(std::pair<std::string, const MCTPDriverHandle*>(driverUUID , handle));
        return true;
    } else {
        return false;
    }
}

bool MCTPEndPointBase::setMCTPTransportBindingIntf(MCTPTransportBindingIntf* mctpTransBindingIntf)
{
    if (_mctpTransBindingIntf != nullptr)
        return false;
    _mctpTransBindingIntf = mctpTransBindingIntf;
    _mctpTransBindingIntf->bindMCTPEndPoint(this);

    // Collect data...
    this->getMessageSupport();
    this->getVendorDefinedMessageSupport();

    this->dumpMsgTypes();
    this->dumpPciVdmMsgTypes();
    this->dumpIanaVdmMsgTypes();
    return true;
}

MCTPTransportBindingIntf* MCTPEndPointBase::getMCTPTransportBindingIntf() const
{
    return _mctpTransBindingIntf;
}

std::uint8_t MCTPEndPointBase::getInstanceId() {
    _iid_mtx.lock();
    if (_instanceId >= 0x1F)
        _instanceId = 0;
    else
        _instanceId ++;
    _iid_mtx.unlock();
    return _instanceId;
}


bool MCTPEndPointBase::isMessageTypeSupported(const std::uint8_t typeId) const {
    return std::find(_msgTypes.begin(), _msgTypes.end(), typeId) != _msgTypes.end();
}

bool MCTPEndPointBase::isVDMTypeSupported(const struct PciVdmMsgTypeId typeId) const {
    return std::find(_PciVdmMsgTypes.begin(), _PciVdmMsgTypes.end(), typeId) != _PciVdmMsgTypes.end();
}

bool MCTPEndPointBase::isVDMTypeSupported(const struct IanaVdmMsgTypeId typeId) const {
    return std::find(_IanaVdmMsgTypes.begin(), _IanaVdmMsgTypes.end(), typeId) != _IanaVdmMsgTypes.end();
}
 
MCTPTransportBindingIntf::MCTPTransportBindingIntf(enum MCTPTransportBindingType bindingType)
    :_bindingType(bindingType), _ep(nullptr)
{

}

enum MCTPTransportBindingType MCTPTransportBindingIntf::getBindingType() const {
    return _bindingType;
}

bool MCTPTransportBindingIntf::bindMCTPEndPoint(MCTPEndPointBase* ep) {
    if (_ep != nullptr || ep == nullptr)
        return false;
    _ep = ep;
    return true;
}

const MCTPEndPointBase* MCTPTransportBindingIntf::getMCTPEndPoint(){
    return _ep;
}

MCTPPCIeVDMBindingIntf::MCTPPCIeVDMBindingIntf(const struct PCIePhyAddr phyAddr)
    :_phyAddr(phyAddr), MCTPTransportBindingIntf(MCTPTransportBindingType::PCIE_VDM)
{

}

bool MCTPPCIeVDMBindingIntf::doSendReceive(int socket, const struct MCTPAddrInfo destAddr, MCTPPacketWrap* dataPack){
    // Invliad request buffer
    if (dataPack == nullptr || dataPack->reqData == nullptr)
        return false;
    // Invalid request length
    if (dataPack->reqSize == 0)
        return false;

    return MCTPSocketUtils::doSendReceive(socket, destAddr, dataPack);
}

struct lladdrBDF MCTPPCIeVDMBindingIntf::getlladdrBDF() {
    struct lladdrBDF lladdr = { 0 };
    lladdr.bus = _phyAddr.Bus;
    lladdr.devfunc = (_phyAddr.Dev << 3) + _phyAddr.Func;
    return lladdr;
}

};
