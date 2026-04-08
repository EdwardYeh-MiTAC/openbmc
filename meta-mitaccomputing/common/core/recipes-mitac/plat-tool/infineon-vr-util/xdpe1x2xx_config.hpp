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
#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace phosphor::software::VR
{

class XDPE1x2xxConfigParser
{
    public:
        enum class SectionCode: uint8_t 
        {
            UnprogrammedSpace   = 0x00,
            Trim                = 0x02,
            Config              = 0x04,
            PMBusLoopA          = 0x07,
            PMBusLoopB          = 0x09,
            ConfigPartial       = 0x0A,
            PMBusPartial        = 0x0B,
            SVIDLoopA           = 0x0D,
            SVIDLoopB           = 0x0E,
            SVIDLoopD           = 0x0F,
            Patch               = 0x10,
            SVIDPartial         = 0x11,
            InvalidatedData     = 0xFF,
        };
        struct SectionHeader
        {
            SectionCode HeaderCode;
            uint8_t XV;
            uint8_t CMD;
            uint8_t Loop;
        };
        struct DataHeader
        {
            SectionHeader SectionInfo;
            uint16_t Size;
            uint16_t Size1;
            uint32_t HeaderCrc32;
        };

    public:
        XDPE1x2xxConfigParser();
        bool getPartNumber(std::string& partNumber);
        bool parseConfig(const std::string filename);
        bool validateConfig();
        bool validateConfig(const std::string productName, const std::string productRevision);

        DataHeader* getFirstSection();
        DataHeader* getNextSection(const DataHeader* currentSection); 
        std::uint32_t* getSectionDataPtr(const DataHeader* currentSection, std::uint32_t& size);
        std::uint32_t getConfigSize();  //Shall retrun number of uint32_t elements
        std::uint32_t getConfigSizeInBytes();  //Shall retrun number of uint8_t elements instead of uint32_t.
        bool getConfigTotalChecksum(std::uint32_t& totalChecksum);

    private:

        std::map<std::string, std::string> _attributes;
        std::vector<std::uint32_t> _rawConfigData;
        bool _isValidConfig;
        std::uint32_t _totalChecksum;
       
        void printSectionHeader(const DataHeader* currentSection); 
        bool getSectionHeaderChecksum(const DataHeader* currentSection, std::uint32_t& checksum);
        bool getSectionDataChecksum(const DataHeader* currentSection, std::uint32_t& checksum); 
        bool isValidHeader(const DataHeader* currentSection);

        bool clearConfig();
        bool isComment(std::string data);

        bool isAttribute(std::string data);
        bool storeAttribute(std::string data);
        bool getAttribute(const std::string key, std::string& attribute);

        bool isConfigStart(std::string data);
        bool isConfigData(std::string data);
        bool storeConfigData(std::vector<std::uint32_t>& rawConfig, std::string data);
        bool isConfigEnd(std::string data);
};

}  // namespace phosphor::software::VR

