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
#include "xdpe1x2xx_products.hpp"
#include "xdpe1x2xx_utility.hpp"

#define MFR_REG_WRITE       0xde
#define MFR_REG_READ        0xdf
#define MFR_FW_COMMAND_DATA 0xfd
#define MFR_FW_COMMAND      0xfe

namespace phosphor::software::VR
{

class XDPE1x2xxControllerIntf
{
    public:
        XDPE1x2xxControllerIntf();
        XDPE1x2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
        std::uint8_t getBusNum();
        std::uint8_t getSlaveAddr();

        std::string getProductName();
        std::string getProductRevision();
        bool getTotalChecksum(std::uint32_t& totalChecksum);
        bool getOTPPartitionSizeRemaining(const std::uint8_t partitionNum, std::uint32_t& sizeRemaining);

        bool programEntireConfigFile(const std::string& filename);

        void bindSMBusInterface(const SMBusInterface* smbusInterface);
        void unbindSMBusInterface();
    protected:
        void updateRevisonDependentSetting(std::uint32_t PIPA,
                                    std::uint8_t RPTR,
                                    std::uint32_t RomId,
                                    std::uint32_t XVRA,
                                    std::uint32_t SCPAD);

        std::uint8_t getRPTR();
        virtual bool getScratchPadAddr(std::uint32_t& scpad);
        virtual bool invalidateAllExistingData();
        virtual bool writeSection(const XDPE1x2xxConfigParser::DataHeader* section);
        virtual bool writeDataToScratchpad(const std::uint32_t* data, const std::uint32_t size);
        std::uint32_t estimateSoakTime(const XDPE1x2xxConfigParser::DataHeader* section);
        virtual bool uploadDataToOTP(const XDPE1x2xxConfigParser::DataHeader* section);
        bool inHealthCondition();        
        bool inEmulationMode();
        const std::string MAGIC_RESET_DEVICE = "Device not in regulation";
        bool resetDevice(const std::string magic);

        bool loadConfig(const std::string& filename);
        bool validateConfig();

        const SMBusInterface* getSMBusInterface();
        void setUpdatable(const bool updatable);
        bool isUpdatable();
    private:
        const std::uint8_t _MaxPartitionNum = 3;
        const SMBusInterface* _smbusInterface;

        DeviceID _devId;
        std::uint8_t _busNum;
        std::uint8_t _slaveAddr;

        std::string _productName;
        std::string _productRev;
        std::uint32_t _PIPA;
        std::uint8_t _RPTR;
        std::uint32_t _RomId;
        std::uint32_t _XVRA;
        std::uint32_t _SCPAD;
        std::uint8_t _SoakTimePerByteInMs;
        bool _RegInited;
        bool _inEmulationMode;
        bool _updatable;

        XDPE1x2xxConfigParser _parser;
        
        bool getScratchPadAddrViaMFRCmd(std::uint32_t& scpad);
        bool getCMLStatus(uint8_t& cmlStatus);
};

XDPE1x2xxControllerIntf* createIntf(const SMBusInterface* smbusInterface, const std::uint8_t busNum, const std::uint8_t slaveAddr);
XDPE1x2xxControllerIntf* createIntf(const SMBusInterface* smbusInterface, const std::uint8_t busNum, const std::uint8_t slaveAddr, const DeviceID& devId);

class XDPE152xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE152xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 

class XDPE192xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE192xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 

class XDPE1A2xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE1A2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 

class XDPE1B2xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE1B2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 

class XDPE1C2xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE1C2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 

class XDPE1D2xxControllerIntf: public XDPE1x2xxControllerIntf
{
    public:
        XDPE1D2xxControllerIntf(const DeviceID& devId, const std::uint8_t busNum, const std::uint8_t slaveAddr);
    protected:
}; 


}  // namespace phosphor::software::VR
