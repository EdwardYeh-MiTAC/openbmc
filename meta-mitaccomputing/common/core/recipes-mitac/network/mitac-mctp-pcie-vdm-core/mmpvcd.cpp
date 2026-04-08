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
#include <vector>
#include <iostream>
#include <charconv>
#include <phosphor-logging/lg2.hpp>

#include "ep-mgr.hpp"
using namespace com::mitac_computing::mctp;

const std::string defaultConfig = "/tmp/mmpvcd.json";

void printUsage()
{
    std::cout << "Print Usage" << std::endl;
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
    std::optional<std::string> configFile;
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if ((*it == "-c") && (it + 1 != end))
        {
            std::string_view value = *(it + 1);
            configFile = std::make_optional<std::string>(std::move(value));
        }
        if (*it == "-h")
        {
            printUsage();
            exit(0);
        }
    }
	const struct PCIePhyAddr addr = { 0x18, 0x00, 0x00 };
	/*
	MCTPBusOwner<struct PCIePhyAddr> bo(0x01, 0x1d, addr);	

	MCTPBusOwner<std::uint8_t> bo2(0x01, 0x1d, 0x33);	
	*/	
    // To check mandatory attributes

    lg2::info("Launch EndpointManager");
	EndpointManager test( 0x01, 0x1d, addr);
    test.loadConfig(configFile.value_or(defaultConfig));
	test.startService();
    return 0;
}
