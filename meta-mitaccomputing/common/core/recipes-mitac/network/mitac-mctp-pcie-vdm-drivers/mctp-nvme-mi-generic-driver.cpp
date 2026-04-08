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
#include "mctp-nvme-mi-generic-driver.hpp"
#include <iostream>
#include <stop_token>
#include <thread>

namespace com::mitac_computing::mctp
{
extern "C" MCTPDriverIntf* getDriver() {
    std::cout << "Hello from the shared library!" << std::endl;
    return new NVMEMiDriverIntf();
}

NVMEMiDriverIntf::NVMEMiDriverIntf() {

}

bool NVMEMiDriverIntf::attachDevice(MCTPEndPointBase* mctpEp)
{
    // Use APIs provided by MCTPEndPointBase to construct runtime resource.
    //return new MCTPDriverHandle(this);
    if (isSupported(mctpEp)) {
        const MCTPDriverHandle* handle = new NVMEMIDriverHandle(this, mctpEp);
        mctpEp->installDriverHandle(handle);
        return true;
    }
    return false;
};

bool NVMEMiDriverIntf::isSupported(const MCTPEndPointBase* mctpEp) const {
    if (mctpEp == nullptr)
        return false;

    const std::uint8_t MESSAGE_TYPE_NVME_MI = 4;
    if (mctpEp->isMessageTypeSupported(MESSAGE_TYPE_NVME_MI))
        return true;
    return false;
}

std::string NVMEMiDriverIntf::getDriverUUID() const{
    // Return SHA256 of class name. SHA256(NVMEMiDriverIntf) = 25bcb8fc27e897a334db9900e79261ed43aa4fafc37413bfbcfebc2f95ac5dc9
    return typeid(*this).name();
}


NVMEMIDriverHandle::NVMEMIDriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp)
    :MCTPDriverHandle(drvIntf, mctpEp)
{
}

NVMEMIDriverHandle::~NVMEMIDriverHandle()
{
}

bool NVMEMIDriverHandle::runTask()
{
    std::cout << "Net " << (std::uint16_t) this->getNet() << " EID " << (std::uint16_t) this->getEId() << " runTask: " << getDriverUUID() << std::endl;
    this->getHealthStatusPoll();
    return true;
}

bool NVMEMIDriverHandle::getHealthStatusPoll() {
    MCTPPacket<struct nvme_mi_req> dataPack(true, MCTP_MSG_TYPE_NVME);
    dataPack.verboseLog = true; // Example about how to enable the dump of raw data for send and receive message of this packet.

    struct nvme_mi_req* req = dataPack.getReqData();
    if (req == nullptr)
        return false;

    req->cmd = 0x08;
    req->opcode = 0x01;
    req->dw1_4 = 0x80;

    //MCTPNvmeMiMsgPacket dataPack(0x08, 16);
    if (!_mctpEp->doSendReceive(&dataPack))
        return false;

    if (dataPack.respSize <= 0x04)
        return false;

    return true;
}

/*
template <typename T>
MCTPPacket::MCTPPacket(const bool isMIC, const std::uint8_t msgType)
    :MCTPPacketWrap(isMIC, msgType, sizeof(T))
{
}
*/

};
