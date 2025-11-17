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
#include <mctp-endpoint-base.hpp>
#include <string>
#include <cstdint>

namespace com::mitac_computing::mctp
{
class MCTPDriverHandle;
class MCTPEndPointBase;

struct PCIID
{
    std::uint8_t VenderId;
    std::uint8_t DeviceId;
};

class MCTPDriverIntf
{
    public:
        virtual bool attachDevice(MCTPEndPointBase* mctpEp) = 0;
        virtual bool isSupported(const MCTPEndPointBase* mctpEp) const = 0;
        virtual std::string getDriverUUID() const = 0;

    protected:
        const MCTPEndPointBase* getMCTPEp();
    private:
        const MCTPEndPointBase* _mctpEp;
};

class MCTPDriverHandle
{
    public:
        MCTPDriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp);
        std::string getDriverUUID() const;
        std::uint8_t getNet() const;
        std::uint8_t getEId() const;

        virtual bool runTask() = 0; //Shall not be while true loop...
    protected:
        const MCTPDriverIntf* _drvIntf;
        const MCTPEndPointBase* _mctpEp;
};
};
