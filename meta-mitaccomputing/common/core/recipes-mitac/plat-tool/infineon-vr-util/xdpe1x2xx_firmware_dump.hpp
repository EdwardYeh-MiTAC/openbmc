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
#include "xdpe1x2xx_config.hpp"
#include <map>
#include <string>

namespace phosphor::software::VR
{

class DevMemOp
{
    public:
        DevMemOp(const std::uint32_t dataBaseAddr);
        virtual bool setAddress(const std::uint32_t address);
        virtual bool readUINT32(std::uint32_t& data);
    protected:
        std::uint32_t _DataBaseAddr;
        std::uint32_t _DataPoint;
};

class EmuDevMemOp: public DevMemOp
{
    public:
        EmuDevMemOp(const std::uint32_t dataSectAddr, const std::string config);
        bool loadData(const std::string config);

        bool setAddress(const std::uint32_t address) override;
        bool readUINT32(std::uint32_t& data) override;
    private: 
        XDPE1x2xxConfigParser _parser;
};

class XDPE1x2xxDevMemOp: public DevMemOp
{
    public:
        XDPE1x2xxDevMemOp(const std::uint32_t dataSectAddr, const std::uint32_t busNum, const std::uint32_t slaveAddr);

        bool bindSMBusInterface(const SMBusInterface* smbusInterface);
        bool setAddress(const std::uint32_t address) override;
        bool readUINT32(std::uint32_t& data) override;
    private: 
        XDPE1x2xxConfigParser _parser;
        std::uint32_t _RPTR;
        std::uint32_t _busNum;
        std::uint32_t _slaveAddr;
        const SMBusInterface* _smbusInterface;
};

class XDPE1x2xxOTPDumpIntf
{
    public:
        XDPE1x2xxOTPDumpIntf();
        XDPE1x2xxOTPDumpIntf(const std::string config);
        XDPE1x2xxOTPDumpIntf(const std::uint8_t busNum, const std::uint8_t slaveAddr, const SMBusInterface* smbusInterface);

        bool dump(const std::string config);
        bool getTotalChecksum(std::uint32_t& totalChecksum);
        bool getOTPPartitionSizeRemaining(const std::uint8_t partitionNum, std::uint32_t& sizeRemaining);

    private:
        DevMemOp* _devMemOp;
        std::uint8_t _busNum;
        std::uint8_t _slaveAddr;
        const SMBusInterface* _smbusInterface;

        const SMBusInterface* getSMBusInterface(); 
        std::uint8_t getBusNum();
        std::uint8_t getSlaveAddr();
        std::uint32_t getOTPTopAddr();

        std::string _partNumber;
        const std::uint32_t _OTPTopAddr = 0x10020000;
        const std::uint8_t _MaxPartitionNum = 3;
        bool readDataHeader(const std::uint32_t address, XDPE1x2xxConfigParser::DataHeader& header);
        bool isValidHeader(const XDPE1x2xxConfigParser::DataHeader& currentSection);
};

}
