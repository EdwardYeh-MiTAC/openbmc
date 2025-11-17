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
#include "brcm-vdm-storelib7-driver.hpp"
#include <iostream>
#include <stop_token>
#include <thread>

namespace com::mitac_computing::mctp
{
extern "C" MCTPDriverIntf* getDriver() {
    std::cout << "Hello from the shared library!" << std::endl;
    return new BRCMStorelib7DriverIntf();
}

BRCMStorelib7DriverIntf::BRCMStorelib7DriverIntf() {
    _VDMMsgTypeSupportList = {
        { 0x1000, 0x10e2 }
    };
}

bool BRCMStorelib7DriverIntf::attachDevice(MCTPEndPointBase* mctpEp)
{
    // Use APIs provided by MCTPEndPointBase to construct runtime resource.
    //return new MCTPDriverHandle(this);
    if (isSupported(mctpEp)) {
        const MCTPDriverHandle* handle = new BRCMStorelib7DriverHandle(this, mctpEp);
        mctpEp->installDriverHandle(handle);
        return true;
    }
    return false;
};

bool BRCMStorelib7DriverIntf::isSupported(const MCTPEndPointBase* mctpEp) const {
    if (mctpEp == nullptr)
        return false;

    for (struct PciVdmMsgTypeId msgId : _VDMMsgTypeSupportList) {
        if (mctpEp->isVDMTypeSupported(msgId))
            return true;
    }
    return false;
}

std::string BRCMStorelib7DriverIntf::getDriverUUID() const{
    // Return SHA256 of class name. SHA256(BRCMStorelib7DriverIntf) = 25bcb8fc27e897a334db9900e79261ed43aa4fafc37413bfbcfebc2f95ac5dc9
    return typeid(*this).name();
}


BRCMStorelib7DriverHandle::BRCMStorelib7DriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp)
    :MCTPDriverHandle(drvIntf, mctpEp)
{
}

BRCMStorelib7DriverHandle::~BRCMStorelib7DriverHandle()
{
}

bool BRCMStorelib7DriverHandle::runTask()
{
    std::cout << "Net " << (std::uint16_t) this->getNet() << " EID " << (std::uint16_t) this->getEId() << " runTask: " << getDriverUUID() << std::endl;
    return true;
}



};
