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
#include <string>
#include <iomanip>
#include <map>

namespace phosphor::software::VR
{

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}


uint32_t calcCRC32(const uint32_t* data, int len);
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

class SMBusInterface
{
    public:
        explicit SMBusInterface();
        ~SMBusInterface() = default;
        virtual bool BLOCK_WRITE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint32_t data) const = 0;
        virtual bool WRITE_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint8_t data) const = 0;
        virtual bool READ_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint8_t& data) const = 0;
        virtual bool SEND_BYTE(const std::uint8_t bus, const std::uint8_t addr, const std::uint8_t data) const = 0;
        virtual bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint32_t& data) const = 0;
        virtual bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint16_t& data) const = 0;
};

class DummySMBusInterface: public SMBusInterface
{
    public:
        explicit DummySMBusInterface();
        ~DummySMBusInterface() = default;
        bool BLOCK_WRITE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint32_t data) const override;
        bool WRITE_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint8_t data) const override;
        bool READ_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint8_t& data) const override;
        bool SEND_BYTE(const std::uint8_t bus, const std::uint8_t addr, const std::uint8_t data) const override;
        bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint32_t& data) const override;
        bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint16_t& data) const override;
};


class LinuxSMBusInterface: public SMBusInterface
{
    public:
        explicit LinuxSMBusInterface();
        ~LinuxSMBusInterface();
        bool BLOCK_WRITE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint32_t data) const override;
        bool WRITE_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint8_t data) const override;
        bool READ_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint8_t& data) const override;
        bool SEND_BYTE(const std::uint8_t bus, const std::uint8_t addr, const std::uint8_t data) const override;
        bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint32_t& data) const override;
        bool BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint16_t& data) const override;
    private:
        mutable std::map<std::uint16_t, int> _i2cDevHandlers;

        int getI2CDevHandler(const std::uint8_t bus, const std::uint8_t slaveAddress) const;
};

struct DeviceID
{
    uint8_t RevisionId;
    uint8_t ProductId;
};
bool getDeviceId(const SMBusInterface* smbusInterface, const std::uint8_t busNum, const std::uint8_t slaveAddr, DeviceID& devId);

void dbg_print_formated_data(std::stringstream& stream, const std::uint8_t slaveAddress, const std::uint8_t command, const std::uint8_t reqSize, const std::uint32_t reqData, const std::uint8_t respSize, const std::uint32_t respData);
void dbg_print_formated_data(std::stringstream& stream, const std::uint8_t slaveAddress, const std::uint8_t reqSize, const std::uint32_t reqData, const std::uint8_t respSize, const std::uint32_t respData);
}
