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
#include "xdpe1x2xx_utility.hpp"
#include <iomanip>
#include <iostream>
#include <thread>
#include <typeinfo>
#include <chrono>

namespace phosphor::software::VR
{

XDPE1x2xxControllerIntf* createIntf(const SMBusInterface* smbusInterface, const std::uint8_t busNum, const std::uint8_t slaveAddr, const DeviceID& devId)
{
    XDPE1x2xxControllerIntf* ctrlIntf = nullptr;

    std::string group = XDPE1x2xxProductMap.at(devId.ProductId);
    group.replace(7 ,2, "xx");
    std::cout << "Group: " << group << std::endl;
    if (group == "XDPE152xx")
    {
        ctrlIntf = new XDPE152xxControllerIntf(devId, busNum, slaveAddr);    
    }
    else if (group == "XDPE192xx")
    {
        ctrlIntf = new XDPE192xxControllerIntf(devId, busNum, slaveAddr);    
    }
    else if (group == "XDPE1A2xx")
    {
        ctrlIntf = new XDPE1A2xxControllerIntf(devId, busNum, slaveAddr);    
    }
    else if (group == "XDPE1B2xx")
    {
        ctrlIntf = new XDPE1B2xxControllerIntf(devId, busNum, slaveAddr);
    }
    else if (group == "XDPE1C2xx")
    {
        ctrlIntf = new XDPE1C2xxControllerIntf(devId, busNum, slaveAddr);
    }
    else if (group == "XDPE1D2xx")
    {
        ctrlIntf = new XDPE1D2xxControllerIntf(devId, busNum, slaveAddr);
    }
    else
    {
        // May only provides very basic function.
        std::cerr << "Unsupported product group: " << group << std::endl;
        ctrlIntf = new XDPE1x2xxControllerIntf(devId, busNum, slaveAddr);
    }
    
    if (ctrlIntf != nullptr)
    {
        ctrlIntf->bindSMBusInterface(smbusInterface);
    }
    return ctrlIntf;
}

XDPE1x2xxControllerIntf* createIntf(const SMBusInterface* smbusInterface, const std::uint8_t busNum, std::uint8_t slaveAddr)
{
    std::cout << "Probe device..." << std::endl;
    DeviceID devId = {0x0, 0x0};
    getDeviceId(smbusInterface, busNum, slaveAddr, devId);

    if (!XDPE1x2xxProductMap.contains(devId.ProductId))
    {
        std::stringstream stream;
        stream << "Unsupported ProductId: 0x" << std::hex << (int) devId.ProductId << std::endl;
        std::cerr << stream.str() << std::endl;
        return nullptr;
    }

    return createIntf(smbusInterface, busNum, slaveAddr, devId);
}

XDPE1x2xxControllerIntf::XDPE1x2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :_devId(devId), _RegInited(false), _smbusInterface(nullptr), _busNum(busNum), _slaveAddr(slaveAddr), _SoakTimePerByteInMs(3), _inEmulationMode(false), _updatable(false)
{
        try {
            std::string productName = XDPE1x2xxProductMap.at(devId.ProductId);
            _productName = productName;
            _productRev = 'A' + devId.RevisionId;
            std::cout << "ProductId: " << std::to_string(devId.ProductId) << " DeviceId: " << std::to_string(devId.RevisionId) << std::endl;
            std::cout << "Initialize controller interface for " << _productName << " Rev: " << _productRev << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
}

std::string XDPE1x2xxControllerIntf::getProductName()
{
    return _productName;
}

void XDPE1x2xxControllerIntf::setUpdatable(const bool updatable)
{
    if (updatable)
        std::cout << "**" << getProductName() << getProductRevision() << " has activated the function of firmware update." << std::endl;

    _updatable = updatable;
}

bool XDPE1x2xxControllerIntf::isUpdatable()
{
    return _updatable;
}

bool XDPE1x2xxControllerIntf::getTotalChecksum(std::uint32_t& totalChecksum)
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

bool XDPE1x2xxControllerIntf::resetDevice(const std::string magic)
{
    std::cout << "resetDevice: Enter." << std::endl;
    if (magic != MAGIC_RESET_DEVICE)
    {
        std::cerr << "Invalid magic. Should be " << std::quoted(MAGIC_RESET_DEVICE) << std::endl;
        return false;
    }

    const SMBusInterface* smbusIntf = getSMBusInterface();
    if (smbusIntf == nullptr)
        return false;

    try
    {
        const std::uint8_t MFR_FW_SUBCMD_RESET = 0x0e;
        if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, MFR_FW_SUBCMD_RESET))
            throw std::runtime_error("Failed to sent reset.");

        const std::uint16_t processTimeReset = 500;
        std::this_thread::sleep_for(std::chrono::milliseconds(processTimeReset));

        std::cout << "resetDevice: Success." << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxControllerIntf::getOTPPartitionSizeRemaining(const std::uint8_t partitionNum, std::uint32_t& sizeRemaining)
{
    std::cout << "getOTPPartitionSizeRemaining: Enter." << std::endl;
    if (inEmulationMode())
    {
        const std::uint32_t OTPSizeForEmulation = 2 * 1024;
        std::cout << "Return " << std::to_string(OTPSizeForEmulation) << " remaining size for emulation." << std::endl;
        sizeRemaining = OTPSizeForEmulation;
        return true;
    }


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

std::string XDPE1x2xxControllerIntf::getProductRevision()
{
    return _productRev;
}

std::uint8_t XDPE1x2xxControllerIntf::getBusNum()
{
    return _busNum;
}

std::uint8_t XDPE1x2xxControllerIntf::getSlaveAddr()
{
    return _slaveAddr;
}

std::uint8_t XDPE1x2xxControllerIntf::getRPTR()
{
    return _RPTR;
}

void XDPE1x2xxControllerIntf::bindSMBusInterface(const SMBusInterface* smbusInterface)
{
    _smbusInterface = smbusInterface;

    try
    {
        const DummySMBusInterface* pDummySMBusInterface = dynamic_cast<const DummySMBusInterface*> (smbusInterface);
        // Assume Emulation only use DummySMBusInterface so we use casting to check whether in emulation mode.
        // It either throw bad_cast or returns nullptr when casting failed.
        if (pDummySMBusInterface != nullptr)
        {
            // Successfully cast to const DummySMBusInterface*
            std::cout << "*** In Emulation Mode ***" << std::endl;
            _inEmulationMode = true;
        }
        else
        {
            _inEmulationMode = false;
        }
    }
    catch(const std::bad_cast& e)
    {
        _inEmulationMode = false;
    }
}

const SMBusInterface* XDPE1x2xxControllerIntf::getSMBusInterface()
{
    return _smbusInterface;
}

void XDPE1x2xxControllerIntf::unbindSMBusInterface()
{
    _smbusInterface = nullptr;
}

bool XDPE1x2xxControllerIntf::inEmulationMode()
{
    return _inEmulationMode;
}

bool XDPE1x2xxControllerIntf::invalidateAllExistingData()
{
    // This feature is not available for revisions A, B, or C for XDPE152xx and revision A for XDPE192xx
    const SMBusInterface* smbusIntf = getSMBusInterface();
    if (smbusIntf == nullptr)
        return false;
   
    std::uint32_t cmd_data = 0x0000fefe;
    if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, cmd_data))
        return false;

    std::uint8_t cmd = 0x12;
    if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, cmd))
        return false;
    
    // The 200ms soak time for invalidate all existing data should be enough.
    const std::uint16_t soakTimeForInvalidateAll = 200;
    std::this_thread::sleep_for(std::chrono::milliseconds(soakTimeForInvalidateAll));
    return true;
}

bool XDPE1x2xxControllerIntf::writeDataToScratchpad(const std::uint32_t* data, const std::uint32_t size)
{
    std::cout << "writeDataToScratchpad: To upload " << std::to_string(size) << " uint32 entries to scratchpad." << std::endl;
    std::uint32_t scpad;
    try
    {
        if (!getScratchPadAddr(scpad))
            throw std::runtime_error("Unable to get scratchpad address.");
        
        const SMBusInterface* smbusIntf = getSMBusInterface();
        if (smbusIntf == nullptr)
            throw std::runtime_error("smbusIntf is null.");
        
        if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), getRPTR(), scpad))
            throw std::runtime_error("Unable to write scratchpad address to register point.");

        for (uint32_t offset = 0; offset < size; offset++)
        {
            if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_REG_WRITE, *data))
                throw std::runtime_error("Failed to write data to register.");
            // Next data
            data ++;
        }
        std::cout << "writeDataToScratchpad: Success." << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

std::uint32_t XDPE1x2xxControllerIntf::estimateSoakTime(const XDPE1x2xxConfigParser::DataHeader* section)
{
    std::uint32_t soakTime = section->Size * _SoakTimePerByteInMs;
    std::cout << "estimateSoakTime: Suggest " << std::to_string(soakTime) << " mSec" << std::endl;
    return soakTime;
}

bool XDPE1x2xxControllerIntf::uploadDataToOTP(const XDPE1x2xxConfigParser::DataHeader* section)
{
    std::cout << "uploadDataToOTP: To commit " << std::to_string(section->Size) << " bytes of data from scratchpad to OTP" << std::endl;
    try
    {
        const SMBusInterface* smbusIntf = getSMBusInterface();
        if (smbusIntf == nullptr)
            throw std::runtime_error("smbusIntf is null.");

        const std::uint8_t PMBUS_CMD_PAGE = 0x00;
        const std::uint8_t bitmaskClearFault = 0x03;
        const std::uint8_t max_page = 2;
        for (std::uint8_t current_page = 0; current_page < max_page; current_page++)
        {
            if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), PMBUS_CMD_PAGE, current_page))
                throw std::runtime_error("Failed to setup page.");
            if (!smbusIntf->SEND_BYTE(getBusNum(), getSlaveAddr(), bitmaskClearFault))
                throw std::runtime_error("Failed to clear fault.");
        }

        if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, section->Size))
            throw std::runtime_error("Failed to setup data size to MFR_FW_COMMAND_DATA.");

        const std::uint8_t MFR_FW_SUBCMD_OTP_CONFIG_STORE = 0x11;
        if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, MFR_FW_SUBCMD_OTP_CONFIG_STORE))
            throw std::runtime_error("Failed to issue MFR_FW_SUBCMD_OTP_CONFIG_STORE command.");

        std::uint32_t soakTimeInMs = estimateSoakTime(section);
        std::this_thread::sleep_for(std::chrono::milliseconds(soakTimeInMs));

        std::cout << "uploadDataToOTP: Success " << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxControllerIntf::getScratchPadAddrViaMFRCmd(std::uint32_t& scpad)
{
    std::string errorMsg;
    std::cout << "ScratchPadAddrViaMFRCmd: Try to query scratchpad address from MFR FW command." << std::endl;
    try
    {
        const SMBusInterface* smbusIntf = getSMBusInterface();
        if (smbusIntf == nullptr)
            throw std::runtime_error("smbusIntf is null");

        // Use MFR_FW command to get scpad address. The address can be different across different models.
        const std::uint32_t FW_ADDRESS_SCPAD_ADDR = 0x00000002;
        errorMsg = "Failed to issue MFR_FW_COMMAND to get scratchpad address.";
        if (!smbusIntf->BLOCK_WRITE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, FW_ADDRESS_SCPAD_ADDR))
            throw std::runtime_error(errorMsg);
        const std::uint8_t MFR_FW_SUBCMD_GET_FW_ADDR = 0x2e;
        if (!smbusIntf->WRITE_BYTE(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND, MFR_FW_SUBCMD_GET_FW_ADDR))
            throw std::runtime_error(errorMsg);

        const std::uint16_t processTimeGetSCPAD = 2;
        std::this_thread::sleep_for(std::chrono::milliseconds(processTimeGetSCPAD));

        // Still fill default data.
        scpad = _SCPAD;
        if (!smbusIntf->BLOCK_READ(getBusNum(), getSlaveAddr(), MFR_FW_COMMAND_DATA, scpad))
            throw std::runtime_error("Get scratchpad address failed.");
        
        std::cout << "ScratchPadAddrViaMFRCmd: Success." << std::endl;
        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxControllerIntf::getScratchPadAddr(std::uint32_t& scpad)
{
    std::string group = getProductName();
    group.replace(7 ,2, "xx");
    if (group == "XDPE152xx")
    {
        scpad = _SCPAD;
        return true;
    }
    if ((group == "XDPE192xx") && (getProductRevision() == "A"))
    {
        scpad = _SCPAD;
        return true;
    }
    
    return getScratchPadAddrViaMFRCmd(scpad); 
}

void XDPE1x2xxControllerIntf::updateRevisonDependentSetting(std::uint32_t PIPA,
                                    std::uint8_t RPTR,
                                    std::uint32_t RomId,
                                    std::uint32_t XVRA,
                                    std::uint32_t SCPAD)
{
    _PIPA = PIPA;
    _RPTR = RPTR;
    _RomId = RomId;
    _XVRA = XVRA;
    _SCPAD = SCPAD;
    std::cout << "VR: " << getProductName() << " Rev: " << getProductRevision() << std::endl;
    std::cout << "PIPA = " << int_to_hex(PIPA) << std::endl;
    std::cout << "RPTR = " << std::to_string(RPTR) << std::endl;
    std::cout << "RomId = " << int_to_hex(RomId) << std::endl;
    std::cout << "XVRA = " << int_to_hex(XVRA) << std::endl;
    std::cout << "SCPAD = " << int_to_hex(SCPAD) << std::endl;
    _RegInited = true;
}

bool XDPE1x2xxControllerIntf::loadConfig(const std::string& filename)
{
    return _parser.parseConfig(filename);
}

bool XDPE1x2xxControllerIntf::validateConfig()
{
    std::string partNumber;
    bool validConfig = false;

    // To confirm compatibility between device and configuration file.
    if (_parser.getPartNumber(partNumber))
        validConfig = _parser.validateConfig(getProductName(), getProductRevision());
    else
        validConfig = _parser.validateConfig();

    if (validConfig)
    {
        XDPE1x2xxConfigParser::DataHeader* section = _parser.getFirstSection();
        while (section != nullptr)
        {
            std::uint32_t size = 0;
            std::uint32_t* data = nullptr;
            data = _parser.getSectionDataPtr(section, size);
            std::cout << "Data Prt: " << int_to_hex(*data) << ", Size: " << std::to_string(size) << std::endl;
            section = _parser.getNextSection(section);
        }
        return true;
    }
    else
    {
        std::cerr << "Invalid configuration." << std::endl;
        return false;
    }
}

bool XDPE1x2xxControllerIntf::getCMLStatus(uint8_t& cmlStatus)
{
    const SMBusInterface* smbusIntf = getSMBusInterface();
    if (smbusIntf == nullptr)
        return false;

    if (!smbusIntf->READ_BYTE(getBusNum(), getSlaveAddr(), 0x7e, cmlStatus))
        return false;
    return true;
}

bool XDPE1x2xxControllerIntf::inHealthCondition()
{
    if (inEmulationMode())
    {
        std::cout << "Make fake health condition for emulation." << std::endl;
        return true;
    }

    const uint8_t MEMORY_FAULT_STATUS = 0x01;
    uint8_t cmlStatus = 0;
    if (!getCMLStatus(cmlStatus) ||
        ((cmlStatus & MEMORY_FAULT_STATUS) == MEMORY_FAULT_STATUS))
    {
        std::cerr << "CML Register indicate MEMORY_FAULT_STATUS." << std::endl;
        return false;
    }
    return true;
}

bool XDPE1x2xxControllerIntf::writeSection(const XDPE1x2xxConfigParser::DataHeader* section)
{
    uint8_t retryRemain = 3;
    uint8_t cmlStatus = 0;
    while (retryRemain--) {
        if (writeDataToScratchpad((const std::uint32_t*) section, section->Size/sizeof(std::uint32_t)) &&
            uploadDataToOTP(section) &&
            inHealthCondition())
        {
            return true;
        }
        std::cout << "Attempts remain: " << std::to_string(retryRemain) << std::endl;
    } 
    std::cerr << "writeSection Failed." << std::endl;
    return false;
}

bool XDPE1x2xxControllerIntf::programEntireConfigFile(const std::string& filename)
{
    if (!isUpdatable())
    {
        std::cerr << "This device doesn't support firmware update." << std::endl; 
        return false;
    }

    if (!loadConfig(filename) || !validateConfig())
    {
        std::cerr << "Unable to program VR FW due to invalid config file." << std::endl;
        return false;
    }

    if (!inHealthCondition())
    {
        std::cerr << "Drop the firmware update due to abnormal condition of VR controller." << std::endl;
        return false;
    }

    if (!inEmulationMode())
    {
        std::uint32_t deviceTotalChecksum = 0;
        std::uint32_t configTotalChecksum = 0;
        if (!getTotalChecksum(deviceTotalChecksum) || !_parser.getConfigTotalChecksum(configTotalChecksum))
        {
            std::cerr << "Unable to get device total checksum." << std::endl;
            return false;
        }

        if (deviceTotalChecksum == configTotalChecksum)
        {
            std::cout << "If the device already has the same checksum (version), then return success to prevent OTP be consumed." << std::endl;
            return true;
        }
    }

    // Make sure available OTP free space can cover the new firmware.
    const std::uint8_t partitionNum = 0;
    std::uint32_t otpRemainSize = 0;
    const std::uint32_t firmwareSize = _parser.getConfigSizeInBytes();
    if (!getOTPPartitionSizeRemaining(partitionNum, otpRemainSize) || (otpRemainSize < firmwareSize))
    {
        std::cerr << "Required size: " << std::to_string(firmwareSize) << ", Available on OTP: " << std::to_string(otpRemainSize) << std::endl;
        return false;
    }
    std::cout << "Available size " << std::to_string(otpRemainSize) << " bytes on OTP is enough for new firmware." << std::endl;

    if (firmwareSize == 0)
    {
        // Found a corner case that it is valid config when firmware image contains empty configuration data with correct total checksum (0x00000000).
        std::cerr << "Firmware size should not 0." << std::endl;
        return false;
    }

    // Below step will invalid the current running firmware.
    // Make sure the firmware image is valid and the device is in normal condition before doing the below step.
    if (!invalidateAllExistingData())
    {
        std::cerr << "Unable to program VR FW from invalidateAllExistingData." << std::endl;
        return false;
    }

    XDPE1x2xxConfigParser::DataHeader* section = _parser.getFirstSection();
    while (section != nullptr)
    {
        if (!writeSection(section))
            return false;
        section = _parser.getNextSection(section);
    }

    //Issue this command when VR is not in regulation.
    resetDevice(MAGIC_RESET_DEVICE);

    return true;
}


XDPE192xxControllerIntf::XDPE192xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE192xxControllerIntf:" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x7000008c, 0xce, 0x5fbd9788, 0x2005bc5c, 0x2005e000);
            setUpdatable(false);
            break;
        case 1: // Revision B
            updateRevisonDependentSetting(0x70000080, 0xce, 0x6192ee1e, 0x2005affc, 0x2005e000);
            setUpdatable(true);
            break;
        case 2: // Revision C
            updateRevisonDependentSetting(0x70000084, 0xce, 0x63cef57d, 0x2005b068, 0x2005e400);
            setUpdatable(true);
            break;
        case 3: // Revision D
            updateRevisonDependentSetting(0x70000084, 0xce, 0x651eeede, 0x2005bca0, 0x2005f000);
            setUpdatable(true);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}

XDPE152xxControllerIntf::XDPE152xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE152xxControllerIntf" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x7000008c, 0xfd, 0x5eea7ae9, 0x2005c2b4, 0x2005e000);
            setUpdatable(false);
            break;
        case 1: // Revision B
            updateRevisonDependentSetting(0x7000008c, 0xfd, 0x5eea7ae9, 0x2005c2b4, 0x2005e000);
            setUpdatable(false);
            break;
        case 2: // Revision C
            updateRevisonDependentSetting(0x7000008c, 0xcd, 0x5fbd9788, 0x2005bc5c, 0x2005e000);
            setUpdatable(false);
            break;
        case 3: // Revision D
            updateRevisonDependentSetting(0x7000008c, 0xcd, 0x60fadc18, 0x20059a24, 0x2005e000);
            setUpdatable(false);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}

XDPE1A2xxControllerIntf::XDPE1A2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE1A2xxControllerIntf" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x70000078, 0xce, 0x60ce0678, 0x2005afe0, 0x2005e000);
            setUpdatable(true);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}

XDPE1B2xxControllerIntf::XDPE1B2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE1B2xxControllerIntf" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x70000000, 0xce, 0x645bd692, 0x2005b408, 0x2005d400);
            setUpdatable(true);
            break;
        case 1: // Revision B
            updateRevisonDependentSetting(0x70000000, 0xce, 0x65737e34, 0x2005af48, 0x2005d000);
            setUpdatable(true);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}

XDPE1C2xxControllerIntf::XDPE1C2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE1C2xxControllerIntf" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x70000000, 0xce, 0x6539363d, 0x2005ae20, 0x2005d400);
            setUpdatable(true);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}

XDPE1D2xxControllerIntf::XDPE1D2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr)
    :XDPE1x2xxControllerIntf(devId, busNum, slaveAddr)
{
    std::cout << "Initialize XDPE1D2xxControllerIntf" << std::endl;
    switch (devId.RevisionId)
    {
        case 0: // Revision A
            updateRevisonDependentSetting(0x70000084, 0xce, 0x645bd692, 0x2005b408, 0x2005d400);
            setUpdatable(true);
            break;
        case 1: // Revision B
            updateRevisonDependentSetting(0x70000084, 0xce, 0x64666bc9, 0x2005bcc0, 0x2005f000);
            setUpdatable(true);
            break;
        default:
            std::cerr << "Unsupported revision" << std::endl;
            setUpdatable(false);
    }
}


}
