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
#pragma once
#include <mctp-driver.hpp>

namespace com::mitac_computing::mctp
{
class NVMEMiController;

class BRCMStorelib7DriverIntf: public MCTPDriverIntf
{
    public:
        BRCMStorelib7DriverIntf();
        ~BRCMStorelib7DriverIntf() = default;
        bool attachDevice(MCTPEndPointBase* mctpEp) override;
        bool isSupported(const MCTPEndPointBase* mctpEp) const override;
        std::string getDriverUUID() const override;

    private:
        std::vector<struct PciVdmMsgTypeId> _VDMMsgTypeSupportList;
};

class BRCMStorelib7DriverHandle: public MCTPDriverHandle
{
    public:
        BRCMStorelib7DriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp);
        ~BRCMStorelib7DriverHandle();
        bool runTask();
};

};
