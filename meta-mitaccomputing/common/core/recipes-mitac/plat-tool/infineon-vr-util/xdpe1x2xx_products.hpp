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

namespace phosphor::software::VR
{

const std::map<std::uint8_t, std::string> XDPE1x2xxProductMap = {
    { 0x8A, "XDPE15284"},
    { 0x8C, "XDPE152C4"},
    { 0x90, "XDPE15254"},
    { 0x95, "XDPE19283"},
    { 0x96, "XDPE192C3"},
    { 0x97, "XDPE192A3"},
    { 0x98, "XDPE19284"},
    { 0x99, "XDPE192C4"},
    { 0x9A, "XDPE1A2G4"},
    { 0x9B, "XDPE1A2G7"},
    { 0x9C, "XDPE1A2G3"},
    { 0x9E, "XDPE1A2G5"},
    { 0xA0, "XDPE1B250"},
    { 0xA1, "XDPE1B254"},
    { 0xA2, "XDPE1B258"},
    { 0xA3, "XDPE1B284"},
    { 0xA5, "XDPE1D2G3"},
    { 0xAB, "XDPE1C284"},
    { 0xAC, "XDPE1C2C4"},
    { 0xAE, "XDPE19283"},   //RevD
    { 0xAF, "XDPE192C3"}
};

}
