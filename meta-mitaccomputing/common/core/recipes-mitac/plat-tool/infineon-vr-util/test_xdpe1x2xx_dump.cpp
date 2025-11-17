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
const std::string defaultEmuFile = "./sample.conf";

void printUsage()
{
    std::cout << "XDPE1x2xx VR Firmware Dump Test Utility" << std::endl;
    std::cout << "Purpose: This utility will load a firmware image (config file) to emulate a XDEP1X2XX device." << std::endl;
    std::cout << "         Then we can verify the firmware dump process by the comparison of checksums between input and output image. " << std::endl;
    std::cout << "Output: Unknown_0_0_Ver0x47e6c87a.txt (The DeviceId will be Unknown from emulation.)" << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << "  -h: This helper" << std::endl << std::endl;
    std::cout << "  * Example " << std::endl;
    std::cout << "      ./test_xdpe1x2xx_dump -b 0x5 -a 0x72 -p " << std::quoted(defaultOutputPath) << " -c " << std::quoted(defaultEmuFile) << std::endl;
    std::cout << "  * Optional" << std::endl;
    std::cout << "      -p: Output path (default: " << defaultOutputPath << ")" << std::endl;
    std::cout << "      -c: Config file for emulation (default: " << defaultEmuFile << ")" << std::endl;
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
    std::optional<std::string> emuFile;
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if ((*it == "-p") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            path = std::make_optional<std::string>(std::move(value));
        }
        if ((*it == "-c") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            emuFile = std::make_optional<std::string>(std::move(value));
        }
        if (*it == "-h")
        {
            printUsage();
            exit(0);
        }
    }

    XDPE1x2xxOTPDumpIntf dumpIntf_emu(emuFile.value_or(defaultEmuFile));
    dumpIntf_emu.dump(path.value_or(defaultOutputPath));
    return 0;
}
