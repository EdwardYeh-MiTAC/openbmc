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
#include "xdpe1x2xx_config.hpp"
#include "xdpe1x2xx_utility.hpp"
#include <algorithm> // Required for std::ranges::to
#include <fstream>
#include <iostream>
#include <ostream>
#include <ranges> // Required for std::views::split
#include <sstream>
#include <iomanip>

#define PartNumAttriName "Part Number"
#define CfgChksumAttriName "Configuration Checksum"

#define TAG_COMMENT "//"
#define TAG_ATTRIBUTE ":"
#define TAG_CONFIG_START "[Configuration Data]"
#define TAG_CONFIG_END "[End Configuration Data]"
#define TAG_END_FILE "[End]"

namespace phosphor::software::VR
{

XDPE1x2xxConfigParser::XDPE1x2xxConfigParser()
    :_isValidConfig(false), _totalChecksum(0)
{

}

bool XDPE1x2xxConfigParser::isComment(std::string data)
{
    trim(data);

    if (data.starts_with(TAG_COMMENT))
    {
        return true;
    } 
    else
    {
        return false;
    }
}

bool XDPE1x2xxConfigParser::isAttribute(std::string data)
{
    trim(data);
    if (isComment(data))
        return false;

    if (data.contains(TAG_ATTRIBUTE))
    {
        return true;
    } 
    else
    {
        return false;
    }
}

bool XDPE1x2xxConfigParser::storeAttribute(std::string data)
{
    std::string delimiter = TAG_ATTRIBUTE;
    size_t pos = data.find(delimiter);
    if (pos == std::string::npos)
    {
        std::cerr << "Invalid definition for attribute: " << data << std::endl;
        return false;
    }
    std::string key = data.substr(0, pos);
    trim(key);
    data.erase(0, pos + delimiter.length());
    trim(data);
    std::string value = data;
    _attributes.insert( std::pair<std::string,std::string>(key, value) ); 
    return true;
}

bool XDPE1x2xxConfigParser::isConfigStart(std::string data)
{
    trim(data);
    if (isComment(data))
        return false;

    if (data.contains(TAG_CONFIG_START))
    {
        return true;
    } 
    else
    {
        return false;
    }
}

bool XDPE1x2xxConfigParser::isConfigData(std::string data)
{
    trim(data);
    std::string delimiter = " ";
    size_t pos = data.find(delimiter);
    if (pos == std::string::npos)
    {
        std::cerr << "Invalid definition for attribute: " << data << std::endl;
        return false;
    }
    std::string key = data.substr(0, pos);
    if (key.length() == 3)
        return true;
    else
        return false;
}

bool XDPE1x2xxConfigParser::storeConfigData(std::vector<std::uint32_t>& rawConfig, std::string data)
{
    trim(data);
    std::string delimiter = " ";

    std::istringstream iss(data); // Create an input string stream from the string
    std::string token;

    // Extract tokens separated by whitespace
    std::uint8_t index = 0;
    while (iss >> token) {
        if (index !=0)
        {
            try {
                unsigned long value = std::stoul(token, nullptr, 16); // 16 for hexadecimal base
                rawConfig.push_back(value);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
                return false;
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range: " << e.what() << std::endl;
                return false;
            }
        }
        index ++;
    }
    //std::cout << "Added " << index - 1 << " Elements, Total = " << rawConfig.size() << std::endl;
    return true;
}

bool XDPE1x2xxConfigParser::isConfigEnd(std::string data)
{
    trim(data);
    if (isComment(data))
        return false;

    if (data.contains(TAG_CONFIG_END))
    {
        return true;
    } 
    else
    {
        return false;
    }
}



bool XDPE1x2xxConfigParser::parseConfig(const std::string filename)
{
    std::cout << "XDPE1x2xxConfigParser::parseConfig Enter" << std::endl;
    clearConfig();

    try
    {
        std::ifstream inputFile(filename);
        if (inputFile.is_open())
        {
            std::string line;
            while (std::getline(inputFile, line))
            {
                if (isComment(line))
                {
                    continue;
                }
                else if (isAttribute(line))
                {
                    storeAttribute(line);
                }
                else if (isConfigStart(line))
                {
                    while (std::getline(inputFile, line))
                    {
                        if (isComment(line))
                        {
                            continue;
                        }
                        else if (isConfigEnd(line))
                        {
                            break;
                        }
                        else if (isConfigData(line))
                        {
                            storeConfigData(_rawConfigData, line);
                        }
                        else
                        {
                            std::cerr << "Ignore: " << line << std::endl;
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
            inputFile.close();
        } else {
            std::cerr << "Error opening: " << filename << std::endl;
            return false;
        }
        std::cout << "XDPE1x2xxConfigParser::parseConfig Success" << std::endl;
        return true;
    } 
    catch (const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }
}

bool XDPE1x2xxConfigParser::clearConfig()
{
    _attributes.clear();
    _rawConfigData.clear();
    _isValidConfig = false;
    _totalChecksum = 0;
    return true;
}

bool XDPE1x2xxConfigParser::isValidHeader(const XDPE1x2xxConfigParser::DataHeader* header)
{
    std::uint32_t crc = calcCRC32((const uint32_t*)header, 2);
    if (crc != header->HeaderCrc32)
        return false;
    return true;
}

XDPE1x2xxConfigParser::DataHeader* XDPE1x2xxConfigParser::getFirstSection(){
    XDPE1x2xxConfigParser::DataHeader* rawArrayPtr = (XDPE1x2xxConfigParser::DataHeader*) _rawConfigData.data();
    if (!isValidHeader(rawArrayPtr))
    {
        std::cerr << "Invalid header" <<std::endl;
        return nullptr;
    }
    std::cout << "valid header" <<std::endl;
    return rawArrayPtr;
}

XDPE1x2xxConfigParser::DataHeader* XDPE1x2xxConfigParser::getNextSection(const XDPE1x2xxConfigParser::DataHeader* currentSection)
{
    if ((currentSection == nullptr) || !isValidHeader(currentSection))
        return nullptr;
    
    std::uint32_t offset_in_uint32 = currentSection->Size / sizeof(uint32_t);
    std::uint32_t* ptrNextSection = (std::uint32_t*) currentSection + offset_in_uint32;
    if (ptrNextSection > _rawConfigData.data() + _rawConfigData.size() - 1)
        return nullptr;

    return (XDPE1x2xxConfigParser::DataHeader*) ptrNextSection;
}

void XDPE1x2xxConfigParser::printSectionHeader(const XDPE1x2xxConfigParser::DataHeader* currentSection)
{
    if (!isValidHeader(currentSection))
    {
        std::cerr << "Invalid header" <<std::endl;
    }
    else
    {   
        std::cout << int_to_hex( *(std::uint32_t*) currentSection) << std::endl;
    }
}

bool XDPE1x2xxConfigParser::getSectionHeaderChecksum(const DataHeader* currentSection, std::uint32_t& checksum)
{
    if ((currentSection == nullptr) || !isValidHeader(currentSection))
        return false;

    checksum = currentSection->HeaderCrc32;
    return true;
    
}

std::uint32_t* XDPE1x2xxConfigParser::getSectionDataPtr(const DataHeader* currentSection, std::uint32_t& size)
{
    if ((currentSection == nullptr) || !isValidHeader(currentSection))
        return nullptr;
        
    size = (currentSection->Size - sizeof(DataHeader) - sizeof(currentSection->HeaderCrc32))/sizeof(std::uint32_t);
    return (std::uint32_t *) currentSection + sizeof(DataHeader)/sizeof(std::uint32_t);
}

bool XDPE1x2xxConfigParser::getSectionDataChecksum(const DataHeader* currentSection, std::uint32_t& checksum)
{
    if ((currentSection == nullptr) || !isValidHeader(currentSection))
        return false;

    std::uint32_t offset_in_uint32 = (currentSection->Size / sizeof(uint32_t)) - 1;
    std::uint32_t* ptrChecksum = (std::uint32_t*) currentSection + offset_in_uint32;
    if (ptrChecksum > _rawConfigData.data() + _rawConfigData.size() - 1)
        return false;

    checksum = *ptrChecksum;
    return true;
    
}

bool XDPE1x2xxConfigParser::getAttribute(const std::string key, std::string& attribute)
{
    try {
        attribute = _attributes.at(key);
        return true;
    } catch (const std::out_of_range& e) {
        return false;
    }
    return false;
}

bool XDPE1x2xxConfigParser::getPartNumber(std::string& partNumber)
{
    if (!getAttribute(PartNumAttriName, partNumber))
        return false;
    return true;
}

bool XDPE1x2xxConfigParser::validateConfig(const std::string productName, const std::string productRevision)
{
    std::string partNumber;
    if (!getPartNumber(partNumber))
        return false;

    if (partNumber != productName + productRevision)
        return false;

    return validateConfig();
}

bool XDPE1x2xxConfigParser::getConfigTotalChecksum(std::uint32_t& totalChecksum)
{
    totalChecksum = _totalChecksum;
    return _isValidConfig;
}

bool XDPE1x2xxConfigParser::validateConfig()
{
    std::uint32_t sumedChecksum = 0;
    XDPE1x2xxConfigParser::DataHeader* section = getFirstSection();
    while (section != nullptr)
    {
        std::uint32_t checksum_header = 0;
        std::uint32_t checksum_data = 0;
        printSectionHeader(section);
        if (getSectionHeaderChecksum(section, checksum_header) && getSectionDataChecksum(section, checksum_data))
        {
            std::cout << "Header Checksum: " << int_to_hex(checksum_header) << ", Data Checksum: " << int_to_hex(checksum_data) << std::endl;
            sumedChecksum += checksum_header + checksum_data;
        }
        else
        {
            return false;
        }
        section = getNextSection(section);
    }

    std::string strCfgTotalChecksum;
    if (!getAttribute(CfgChksumAttriName, strCfgTotalChecksum))
        return false;

    try {
        uint32_t u32CfgTotalChecksum = std::stoul(strCfgTotalChecksum, nullptr, 16); // 16 for hexadecimal base
        if (u32CfgTotalChecksum == sumedChecksum)
        {
            std::cout << "Checksum matched." << std::endl;
            _isValidConfig = true;
            _totalChecksum = u32CfgTotalChecksum;
            return true;
        }
        else
        {
            std::cerr << "Expected: " << strCfgTotalChecksum << ", Real: " << int_to_hex(sumedChecksum) << std::endl;
            return false;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return false;
    }
    return false;
}

std::uint32_t XDPE1x2xxConfigParser::getConfigSize()
{
    return _rawConfigData.size();
}

std::uint32_t XDPE1x2xxConfigParser::getConfigSizeInBytes()
{
    return _rawConfigData.size() * sizeof(std::uint32_t);
}

}
