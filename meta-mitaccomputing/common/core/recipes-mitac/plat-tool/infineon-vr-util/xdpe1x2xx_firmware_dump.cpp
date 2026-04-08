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
#include "xdpe1x2xx_controller_intf.hpp"
#include "xdpe1x2xx_firmware_dump.hpp"
#include "xdpe1x2xx_utility.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <thread>

namespace phosphor::software::VR
{

DevMemOp::DevMemOp(const std::uint32_t dataBaseAddr)
    :_DataBaseAddr(dataBaseAddr), _DataPoint(dataBaseAddr)
{

}

bool DevMemOp::setAddress(const std::uint32_t address)
{
    _DataPoint = address;
    std::cout << "DevMemOp: Set Address: " << int_to_hex(address) << std::endl;
    return true;
}

bool DevMemOp::readUINT32(std::uint32_t& data){
    std::cout << "DevMemOp: data[" << int_to_hex(_DataPoint) << "] = " << int_to_hex(data) << std::endl;
    _DataPoint += sizeof(std::uint32_t);
    return true;
}



EmuDevMemOp::EmuDevMemOp(const std::uint32_t dataBaseAddr, const std::string config)
    :DevMemOp(dataBaseAddr)
{
    loadData(config);
}

bool EmuDevMemOp::loadData(const std::string filename)
{
   return _parser.parseConfig(filename); 
}

bool EmuDevMemOp::setAddress(const std::uint32_t address)
{
    if ((address < _DataBaseAddr) || (address > _DataBaseAddr + _parser.getConfigSizeInBytes()))
        return false;

    return DevMemOp::setAddress(address);
}

bool EmuDevMemOp::readUINT32(std::uint32_t& data){
    const std::uint32_t offset = (_DataPoint - _DataBaseAddr) / sizeof(std::uint32_t);
    if ((offset < 0) || (offset >= _parser.getConfigSize()))
    {
        data = 0;
        throw std::out_of_range("Config file doesn't have token to indicate the last of record.  Throw exception to workaround this issue.");
        return false;
    }

    const uint32_t* rawData = (const uint32_t*) _parser.getFirstSection();
    data = rawData[offset];
    return DevMemOp::readUINT32(data);
}



XDPE1x2xxDevMemOp::XDPE1x2xxDevMemOp(const std::uint32_t dataBaseAddr, const std::uint32_t busNum, const std::uint32_t slaveAddr)
    :DevMemOp(dataBaseAddr), _busNum(busNum), _slaveAddr(slaveAddr), _smbusInterface(nullptr), _RPTR(0xce)
{
}

bool XDPE1x2xxDevMemOp::bindSMBusInterface(const SMBusInterface* smbusInterface)
{
    _smbusInterface = smbusInterface;
    return true;
}

bool XDPE1x2xxDevMemOp::setAddress(const std::uint32_t address)
{
    bool status = _smbusInterface->BLOCK_WRITE(_busNum, _slaveAddr, _RPTR, address);
    DevMemOp::setAddress(address);
    return status;
}

bool XDPE1x2xxDevMemOp::readUINT32(std::uint32_t& data)
{
    bool status = _smbusInterface->BLOCK_READ(_busNum, _slaveAddr, MFR_REG_READ, data);
    DevMemOp::readUINT32(data);
    return status;
}


XDPE1x2xxOTPDumpIntf::XDPE1x2xxOTPDumpIntf()
    :_smbusInterface(nullptr)
{

}

XDPE1x2xxOTPDumpIntf::XDPE1x2xxOTPDumpIntf(const std::string config)
    :_busNum(0), _slaveAddr(0), _smbusInterface(nullptr)
{
    _devMemOp = new EmuDevMemOp(getOTPTopAddr(), config);
}

XDPE1x2xxOTPDumpIntf::XDPE1x2xxOTPDumpIntf(const std::uint8_t busNum, const std::uint8_t slaveAddr, const SMBusInterface* smbusInterface)
    :_busNum(busNum), _slaveAddr(slaveAddr), _smbusInterface(smbusInterface)
{
    XDPE1x2xxDevMemOp* devMemOp = new XDPE1x2xxDevMemOp(getOTPTopAddr(), busNum, slaveAddr);
    devMemOp->bindSMBusInterface(smbusInterface); 
    _devMemOp = devMemOp;
}

const SMBusInterface* XDPE1x2xxOTPDumpIntf::getSMBusInterface()
{
    return _smbusInterface;
}

std::uint8_t XDPE1x2xxOTPDumpIntf::getSlaveAddr()
{
    return _slaveAddr;
}

std::uint8_t XDPE1x2xxOTPDumpIntf::getBusNum()
{
    return _busNum;
}

std::uint32_t XDPE1x2xxOTPDumpIntf::getOTPTopAddr()
{
    return _OTPTopAddr;
}

bool XDPE1x2xxOTPDumpIntf::getTotalChecksum(std::uint32_t& totalChecksum)
{
    std::cout << "getTotalChecksum: Enter." << std::endl;
    const SMBusInterface* smbusIntf = getSMBusInterface();
    if (smbusIntf == nullptr)
        return false;

    try
    { 
        // Use MFR_FW command to get crc32 address. The address can be different across different models.
        const std::uint32_t FW_ADDRESS_CRC32_ADDR = 0x00000000;
        std::string errorMsg = "Failed to issue MFR_FW_COMMAND to get totol checksum.";
        if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, FW_ADDRESS_CRC32_ADDR))
            throw std::runtime_error(errorMsg);
        const std::uint8_t MFR_FW_SUBCMD_GET_CRC = 0x2d;
        if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, MFR_FW_SUBCMD_GET_CRC))
            throw std::runtime_error(errorMsg);

        const std::uint16_t processTimeGetCRC32 = 20;
        std::this_thread::sleep_for(std::chrono::milliseconds(processTimeGetCRC32));

        if (!smbusIntf->BLOCK_READ(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, totalChecksum))
            throw std::runtime_error("Get scratchpad address failed.");
        
        std::cout << "getTotalChecksum: Success." << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxOTPDumpIntf::getOTPPartitionSizeRemaining(const std::uint8_t partitionNum, std::uint32_t& sizeRemaining)
{
    std::cout << "getOTPPartitionSizeRemaining: Enter." << std::endl;
    const SMBusInterface* smbusIntf = getSMBusInterface();
    if (smbusIntf == nullptr)
        return false;

    try
    { 
        if (partitionNum >= _MaxPartitionNum)
            throw std::runtime_error("Exceed maximum partition number.");

        // Use MFR_FW command to get crc32 address. The address can be different across different models.
        const std::uint32_t FW_ADDRESS_PARTITION_NUM = partitionNum << 24;
        std::string errorMsg = "Failed to issue MFR_FW_COMMAND to get size of partition.";
        if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, FW_ADDRESS_PARTITION_NUM))
            throw std::runtime_error(errorMsg);
        const std::uint8_t MFR_FW_SUBCMD_OTP_PARTITION_SIZE_REMAINING = 0x10;
        if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, MFR_FW_SUBCMD_OTP_PARTITION_SIZE_REMAINING))
            throw std::runtime_error(errorMsg);

        const std::uint16_t processTimeGetOTPPartitionSizeRemaining = 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(processTimeGetOTPPartitionSizeRemaining));

        if (!smbusIntf->BLOCK_READ(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, sizeRemaining))
            throw std::runtime_error("Get scratchpad address failed.");
        
        std::cout << "getOTPPartitionSizeRemaining: Success." << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxOTPDumpIntf::dump(const std::string outputPath)
{
    const std::uint32_t sizeOfHeaderInU32 = sizeof(XDPE1x2xxConfigParser::DataHeader)/sizeof(uint32_t);
    const std::uint32_t sizeOfCRC32InU32 = 1;

    bool result = false;
    std::string partNumber = "Unknown";
    XDPE1x2xxConfigParser::DataHeader header = {};
    std::map<std::uint32_t, std::uint32_t*> sectionData;
    std::uint32_t headerAddr = getOTPTopAddr();
    std::uint32_t cfgChecksum = 0;

    DeviceID id;
    if ((_smbusInterface != nullptr)
        && getDeviceId(_smbusInterface, _busNum, _slaveAddr, id)
        && XDPE1x2xxProductMap.contains(id.ProductId))
    {
        const std::string productName = XDPE1x2xxProductMap.at(id.ProductId);
        const char revId = 'A' + id.RevisionId;
        partNumber = productName + revId;
    }
    try
    {
        bool stop = false;
        const uint32_t MAX_SECTION_SIZE = 32768;
        while(!stop && readDataHeader(headerAddr, header))
        {
            if (headerAddr - getOTPTopAddr() >= MAX_SECTION_SIZE)
            {
                std::cout << "Pointer is exceed maximum section size. For now, it only support dump data from first section zoom." << std::endl;
                stop = true;
                continue;
            }

            if (!isValidHeader(header))
            {
                throw std::runtime_error("Invalid Header detected.");
            }

            if (header.Size == 0x00)
            {
                stop = true;
                continue;
            }

            switch (header.SectionInfo.HeaderCode)
            {
                case XDPE1x2xxConfigParser::SectionCode::UnprogrammedSpace:
                    stop = true;
                    headerAddr +=  header.Size;
                    continue;
                    break;
                case XDPE1x2xxConfigParser::SectionCode::InvalidatedData:
                    headerAddr +=  header.Size;
                    continue;
                case XDPE1x2xxConfigParser::SectionCode::Trim:
                    headerAddr +=  header.Size;
                    continue;
                case XDPE1x2xxConfigParser::SectionCode::ConfigPartial:
                case XDPE1x2xxConfigParser::SectionCode::PMBusPartial:
                case XDPE1x2xxConfigParser::SectionCode::Patch:
                case XDPE1x2xxConfigParser::SectionCode::SVIDPartial:
                    std::cout << "Partial section code detected. Not support yet.";
                    continue;
                case XDPE1x2xxConfigParser::SectionCode::Config:
                case XDPE1x2xxConfigParser::SectionCode::PMBusLoopA:
                case XDPE1x2xxConfigParser::SectionCode::PMBusLoopB:
                case XDPE1x2xxConfigParser::SectionCode::SVIDLoopA:
                case XDPE1x2xxConfigParser::SectionCode::SVIDLoopB:
                case XDPE1x2xxConfigParser::SectionCode::SVIDLoopD:
                    // Normal case
                    break;
                default:
                    throw std::runtime_error("Unknown SectionCode. Assume the response date from device might be incorrect.");
                    break;
            }
            const std::uint32_t sizeOfSectionInU32 = header.Size / sizeof(std::uint32_t);
            const std::uint32_t sizeOfDataInU32 = sizeOfSectionInU32 - sizeOfHeaderInU32 - sizeOfCRC32InU32;

            std::uint32_t* data = new std::uint32_t[sizeOfSectionInU32];
            // Copy number of bytes
            std::memcpy((std::uint32_t *) data, &header, sizeOfHeaderInU32 * sizeof(std::uint32_t));
            
            const std::uint32_t sectionInfo = *data;
            // Duplicate item may not allowed.
            if (sectionData.contains(sectionInfo))
                throw std::runtime_error("Assume duplicated SectionInfo shall not happen.");
            sectionData[sectionInfo] = data;

            for (std::uint32_t index = sizeOfHeaderInU32; index < sizeOfSectionInU32; index ++)
            {
                if (!_devMemOp->readUINT32(data[index]))
                    throw std::runtime_error("Communication issue with device while reading data.");
            }

            const std::uint32_t dataCrc = data[sizeOfSectionInU32 - 1];   //CRC32 is the last element.
            
            if ( dataCrc != calcCRC32((const uint32_t*)&data[sizeOfHeaderInU32], sizeOfDataInU32) )
            {
                std::string errMsg = "Crc mismatched. Expected: " + int_to_hex(dataCrc) + "\n";
                throw std::runtime_error(errMsg);
            }

            cfgChecksum = cfgChecksum + header.HeaderCrc32 + dataCrc;

            headerAddr +=  header.Size;
        }
        if (stop)
        {
            std::ofstream outputFile;
            std::string fn = outputPath + "/" + partNumber
                                        + "_" + std::to_string(_busNum)
                                        + "_" + std::to_string(_slaveAddr)
                                        + "_Ver" + int_to_hex(cfgChecksum) + ".txt";

            
            outputFile.open(fn);
            if (!outputFile.is_open()) {
                std::string msg = "Failed to open " + fn;
                throw std::runtime_error(msg);
            }

            outputFile << "Created by : xdpe1x2xx_firmware_dump" << std::endl;
            outputFile << "Bus Number : " << std::to_string(_busNum) << std::endl;
            outputFile << "Slave Address : " << std::to_string(_slaveAddr) << std::endl;
            outputFile << "Part Number : " << partNumber << std::endl;

            std::uint32_t deviceTotalChecksum = 0xFFFFFFFF;
            if (getTotalChecksum(deviceTotalChecksum))
            {
                if (deviceTotalChecksum != cfgChecksum)
                    std::cerr << "Total Checksum mismatch. Get from device: " << int_to_hex(deviceTotalChecksum) << " Calculated by tool: " << int_to_hex(cfgChecksum) << std::endl;
                //We should always use the checksum calculated by device.
                outputFile << "Configuration Checksum : " << int_to_hex(deviceTotalChecksum) << std::endl;
            }
            else
            {
                outputFile << "Configuration Checksum : " << int_to_hex(cfgChecksum) << std::endl;
            }
            outputFile << "[Configuration Data]" << std::endl;

            const std::uint8_t NumOfU32PerLine = 4;
            for (const auto& pair : sectionData)
            {
                std::uint32_t* data = pair.second;
                XDPE1x2xxConfigParser::DataHeader* header = (XDPE1x2xxConfigParser::DataHeader*) data;
                std::stringstream stream;
                for (std::uint32_t offset = 0; offset < header->Size / sizeof(std::uint32_t); offset++)
                {
                    // To prompt start of section.
                    if (offset == 0x0)
                        stream << "//Start of Section" << std::endl;

                    // To prompt address
                    if ((offset % NumOfU32PerLine) == 0)
                    {
                        stream << std::setfill ('0') << std::setw(3) 
                               << std::hex << std::uppercase << offset * sizeof(std::uint32_t) << " ";
                    }
                    
                    // To prompt data
                    stream << std::setfill ('0') << std::setw(8) 
                           << std::hex << data[offset] << " ";

                    if ((offset % NumOfU32PerLine) == NumOfU32PerLine - 1)
                        stream << std::endl;
                }
                // In case need to change line for the last of line
                if (header->Size % (NumOfU32PerLine * sizeof(std::uint32_t)) != 0)
                    stream << std::endl;
                    
                outputFile << stream.str();
            }
            outputFile << "[End Configuration Data]" << std::endl;
            outputFile << "[End]" << std::endl;

            result = true;
        }
        else
        {
            throw std::runtime_error("Failed while traverse of headers.");
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        result = false;
    }

    std::uint32_t remainSize = 0;
    for (std::uint32_t pn = 0; pn < _MaxPartitionNum; pn++)
    { 
        if (getOTPPartitionSizeRemaining(pn, remainSize))
            std::cout << "Page " << std::to_string(pn) << " : " << std::to_string(remainSize) << " bytes." << std::endl;
    }
    
    //GC
    for (const auto& pair : sectionData) {
        delete []pair.second;
    }

    return result;
}

bool XDPE1x2xxOTPDumpIntf::readDataHeader(const std::uint32_t address, XDPE1x2xxConfigParser::DataHeader& header)
{

    if ((_devMemOp == nullptr) || (!_devMemOp->setAddress(address)))
        return false;

    const std::uint8_t szDataHeaderInU32 = sizeof(header)/sizeof(std::uint32_t);
    std::uint32_t u8Header[szDataHeaderInU32];
    try
    {
        for (std::uint8_t offset = 0; offset < szDataHeaderInU32; offset++)
        {
            if (!_devMemOp->readUINT32(u8Header[offset]))
            {
                std::cerr << "Read Header failed." << std::endl;
                return false;
            }
        }
        std::memcpy((std::uint32_t *) &header, u8Header, sizeof(header));
        return true;
    }
    catch (const std::out_of_range& e)
    {
        // Fake a valid terminater Header when using EmuDevMemOp with config file for emulation.
        header.SectionInfo.HeaderCode = XDPE1x2xxConfigParser::SectionCode::UnprogrammedSpace;
        header.Size = 0;
        header.Size1 = 0;
        const std::uint8_t szDataHeader = sizeof(header) - sizeof(header.HeaderCrc32);
        header.HeaderCrc32 = calcCRC32((const uint32_t*)&header, szDataHeader/sizeof(std::uint32_t));
        return true;
    }
}

bool XDPE1x2xxOTPDumpIntf::isValidHeader(const XDPE1x2xxConfigParser::DataHeader& header)
{
    // Based on experiment, InvalidatedData and UnprogrammedSpace doesn't provide valid header CRC.
    // However, we should blind trust the header and take the length field.
    if ((header.SectionInfo.HeaderCode == XDPE1x2xxConfigParser::SectionCode::InvalidatedData) ||
        (header.SectionInfo.HeaderCode == XDPE1x2xxConfigParser::SectionCode::UnprogrammedSpace))
        return true;

    const std::uint8_t szDataHeader = sizeof(header) - sizeof(header.HeaderCrc32);
    std::uint32_t crc = calcCRC32((const std::uint32_t*)&header, szDataHeader/sizeof(std::uint32_t));
    if (crc != header.HeaderCrc32)
    {
        std::cerr << "Expected: " << int_to_hex(header.HeaderCrc32) << ", Real: " << int_to_hex(crc) << std::endl;
        return false;
    }
    return true;
}

}
