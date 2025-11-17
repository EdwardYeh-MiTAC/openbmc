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
class SpdmController;

class SpdmDriverIntf: public MCTPDriverIntf
{
    public:
        SpdmDriverIntf();
        ~SpdmDriverIntf() = default;
        bool attachDevice(MCTPEndPointBase* mctpEp) override;
        bool isSupported(const MCTPEndPointBase* mctpEp) const override;
        std::string getDriverUUID() const override;
};

class SpdmDriverHandle: public MCTPDriverHandle
{
    public:
        SpdmDriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp);
        ~SpdmDriverHandle();
        bool runTask();

        bool getVersion(std::uint8_t& maxVer);
        bool getCapability(const std::uint8_t ver, std::uint32_t& cap);
        bool negotiateAlgo(const std::uint8_t ver);
        bool getDigest(const std::uint8_t ver);
        bool getCertificate(const std::uint8_t ver);
        bool challenge(const std::uint8_t ver);
        //bool getMeasurement(const std::uint8_t ver);
        //bool getHealthStatusPoll();
    private:
        bool _inited;
};

};
