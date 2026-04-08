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
#include "xdpe1x2xx_utility.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
extern "C"
{
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
}

namespace phosphor::software::VR
{

const uint32_t CRC32Poly = 0xEDB88320;

uint32_t calcCRC32(const uint32_t* data, int len)
{
    if (data == NULL)
    {
        return 0;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (int b = 0; b < 32; b++)
        {
            if (crc & 0x1)
            {
                crc = (crc >> 1) ^ CRC32Poly; // lsb-first
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

bool getDeviceId(const SMBusInterface* smbusInterface, const std::uint8_t busNum, const std::uint8_t slaveAddr, DeviceID& devId)
{
    const std::uint8_t PMBusCmd_IC_DEVICE_ID = 0xAD;
    if (smbusInterface)
    {
        union DID{
            struct Attr_t{
                std::uint8_t revisonId;
                std::uint8_t productId;
            }attr;
            std::uint16_t raw;
        };

        if (smbusInterface->BLOCK_READ(busNum, slaveAddr, PMBusCmd_IC_DEVICE_ID, (std::uint16_t&)devId))
        {
            return true;
        }
    }
    return true;
}


void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

SMBusInterface::SMBusInterface()
{
}

DummySMBusInterface::DummySMBusInterface()
{
}

bool DummySMBusInterface::BLOCK_WRITE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint32_t data) const
{
    std::stringstream stream;
    stream << "BLOCK_WRITE";
    dbg_print_formated_data(stream, addr, command, 4, data, 0, 0);
    return true;
}

bool DummySMBusInterface::WRITE_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint8_t data) const
{
    std::stringstream stream;
    stream << "WRITE_BYTE";
    dbg_print_formated_data(stream, addr, command, sizeof(data), data, 0, 0);
    return true;
}

bool DummySMBusInterface::READ_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint8_t& data) const
{
    std::stringstream stream;
    stream << "READ_BYTE";
    dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
    return true;
}

bool DummySMBusInterface::SEND_BYTE(const std::uint8_t bus, const std::uint8_t addr, const std::uint8_t data) const
{
    std::stringstream stream;
    stream << "SEND_BYTE";
    dbg_print_formated_data(stream, addr, sizeof(data), data, 0, 0);
    return true;
}

bool DummySMBusInterface::BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint32_t& data) const
{
    std::stringstream stream;
    stream << "BLOCK_READ";
    dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
    return true;
}

bool DummySMBusInterface::BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint16_t& data) const
{
    std::stringstream stream;
    stream << "BLOCK_READ";
    dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
    return true;
}


LinuxSMBusInterface::LinuxSMBusInterface()
{
}

LinuxSMBusInterface::~LinuxSMBusInterface()
{
    // Close all handler.
    for (const auto& pair : _i2cDevHandlers)
    {
        const int fd = pair.second;
        close(fd);
    }
}

int LinuxSMBusInterface::getI2CDevHandler(const std::uint8_t bus, const std::uint8_t slaveAddress) const
{
    const std::uint16_t i2cDevToken = (bus << 8) + slaveAddress;
    std::uint16_t i2cDevHandler = -1;
    if (!_i2cDevHandlers.contains(i2cDevToken))
    {
        int fd = 0;
        std::string fileName = "/dev/i2c-" + std::to_string(bus);

        // Open port for reading and writing
        if ((fd = open(fileName.c_str(), O_RDWR)) < 0)
        {
            return -1;
        }

        // Set the port options and set the address of the device
        if (ioctl(fd, I2C_SLAVE, slaveAddress) < 0)
        {
            close(fd);
            return -1;
        }
        _i2cDevHandlers[i2cDevToken] = fd;
        return fd;
    }
    else
    {
        return _i2cDevHandlers[i2cDevToken];
    }
}

void dbg_print_formated_data(std::stringstream& stream, const std::uint8_t slaveAddress, const std::uint8_t command, const std::uint8_t reqSize, const std::uint32_t reqData, const std::uint8_t respSize, const std::uint32_t respData)
{
    stream << std::hex;
    stream << " ( 0x" << std::setfill ('0') << std::setw(2) << (std::uint32_t) slaveAddress;
    stream << ", 0x" << std::setfill ('0') << std::setw(2) << (std::uint32_t) command;
    if (reqSize != 0)
        stream << ", 0x" << std::setfill ('0') << std::setw(reqSize * 2) << reqData;

    stream << " )";

    if (respSize != 0)
        stream << " Resp: ( 0x" << std::setfill ('0') << std::setw(respSize * 2) << respData << " )";

    std::cout << stream.str() << std::endl;;
}

void dbg_print_formated_data(std::stringstream& stream, const std::uint8_t slaveAddress, const std::uint8_t reqSize, const std::uint32_t reqData, const std::uint8_t respSize, const std::uint32_t respData)
{
    stream << std::hex;
    stream << " ( 0x" << std::setfill ('0') << std::setw(2) << (std::uint32_t) slaveAddress;
    if (reqSize != 0)
        stream << ", 0x" << std::setfill ('0') << std::setw(reqSize * 2) << reqData;

    stream << " )";

    if (respSize != 0)
        stream << " Resp: ( 0x" << std::setfill ('0') << std::setw(respSize * 2) << respData << " )";

    std::cout << stream.str() << std::endl;
}

bool LinuxSMBusInterface::BLOCK_WRITE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint32_t data) const
{
    std::stringstream stream;
    stream << "BLOCK_WRITE";
    dbg_print_formated_data(stream, addr, command, 4, data, 0, 0);
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const int resp_len = i2c_smbus_write_block_data(fd, command, sizeof(data), (const std::uint8_t*) &data);
        if (resp_len < 0)
        {
            throw std::runtime_error("i2c_smbus_write_block_data failed.");
        }
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool LinuxSMBusInterface::WRITE_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, const std::uint8_t data) const
{
    std::stringstream stream;
    stream << "WRITE_BYTE";
    dbg_print_formated_data(stream, addr, command, sizeof(data), data, 0, 0);
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const int status = i2c_smbus_write_byte_data(fd, command, data);
        if (status < 0)
            throw std::runtime_error("i2c_smbus_write_byte_data failed.");

        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool LinuxSMBusInterface::READ_BYTE(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint8_t& data) const
{
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const int resp = i2c_smbus_read_byte_data(fd, command);
        if (resp < 0)
            throw std::runtime_error("i2c_smbus_read_byte_data failed.");

        data = (std::uint8_t) resp;
        std::stringstream stream;
        stream << "READ_BYTE";
        dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool LinuxSMBusInterface::SEND_BYTE(const std::uint8_t bus, const std::uint8_t addr, const std::uint8_t data) const
{
    std::stringstream stream;
    stream << "SEND_BYTE";
    dbg_print_formated_data(stream, addr, sizeof(data), data, 0, 0);
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const int status = i2c_smbus_write_byte(fd, data);
        if (status < 0)
            throw std::runtime_error("i2c_smbus_write_byte failed.");

        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool LinuxSMBusInterface::BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint32_t& data) const
{
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const std::uint8_t len = 32;
        std::uint8_t buffer[len] = {0};
        const int resp_len = i2c_smbus_read_block_data(fd, command, buffer);
        if (resp_len < 0)
            throw std::runtime_error("i2c_smbus_read_block_data failed.");

        if (resp_len != sizeof(data))
            throw std::runtime_error("Invalid response length");

        std::memcpy((std::uint8_t *) &data, buffer, sizeof(data));

        std::stringstream stream;
        stream << "BLOCK_READ";
        dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool LinuxSMBusInterface::BLOCK_READ(const std::uint8_t bus, const std::uint8_t addr, std::uint8_t command, std::uint16_t& data) const
{
    try
    {
        const int fd = getI2CDevHandler(bus, addr);
        if (fd <= 0)
            throw std::runtime_error("getI2CDevHandler failed.");

        const std::uint8_t len = 32;
        std::uint8_t buffer[len] = {0};
        const int resp_len = i2c_smbus_read_block_data(fd, command, buffer);
        if (resp_len < 0)
            throw std::runtime_error("i2c_smbus_read_block_data failed.");

        if (resp_len != sizeof(data))
            throw std::runtime_error("Invalid response length");

        std::memcpy((std::uint8_t *) &data, buffer, sizeof(data));

        std::stringstream stream;
        stream << "BLOCK_READ";
        dbg_print_formated_data(stream, addr, command, 0, 0, sizeof(data), data);
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

}
