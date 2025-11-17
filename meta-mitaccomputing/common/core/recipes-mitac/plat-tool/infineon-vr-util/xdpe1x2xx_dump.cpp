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

#include <iostream>
#include <charconv>

using namespace phosphor::software::VR;

const std::string defaultOutputPath = "/tmp";

void printUsage()
{
    std::cout << "XDPE1x2xx VR Firmware Dump Utility" << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << "  -h: This helper" << std::endl << std::endl;
    std::cout << "  * Example " << std::endl;
    std::cout << "      ./xdpe1x2xx_dump -b 0x5 -a 0x72 -p " << std::quoted(defaultOutputPath) << std::endl;
    std::cout << "  * Mandatory" << std::endl;
    std::cout << "      -b: Bus Number" << std::endl;
    std::cout << "      -a: Slave Address" << std::endl;
    std::cout << "  * Optional" << std::endl;
    std::cout << "      -p: Output path (default: " << defaultOutputPath << ")" << std::endl;
}

bool getOptNumTypeOfArg(std::string_view arg, auto& optData)
{
    std::chars_format format = std::chars_format::general;
    const std::string hexId = "0x";
    if (arg.starts_with(hexId))
    {
        format = std::chars_format::hex;
        arg.remove_prefix(hexId.size());
    }
    double u8Data;
    if (std::from_chars(arg.data(), arg.data() + arg.size(), u8Data, format).ec == std::errc{})
    {
        optData = std::make_optional<std::uint8_t>(u8Data);
        return true;
    }
    return false;
}

int main(int argc, const char *argv[])
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    std::optional<std::uint8_t> busNum, slaveAddr;
    std::optional<std::string> path;
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if ((*it == "-a") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            getOptNumTypeOfArg(value, slaveAddr);
        }
        if ((*it == "-b") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            getOptNumTypeOfArg(value, busNum);
        }
        if ((*it == "-p") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            path = std::make_optional<std::string>(std::move(value));
        }
        if (*it == "-h")
        {
            printUsage();
            exit(0);
        }
    }

    // To check mandatory attributes
    if (!busNum || !slaveAddr)
    {
        printUsage();
        exit(-1);
    }
    const LinuxSMBusInterface smbusInterface;

    XDPE1x2xxOTPDumpIntf dumpIntf(busNum.value(), slaveAddr.value(), &smbusInterface);
    dumpIntf.dump(path.value_or(defaultOutputPath));
    return 0;
}
