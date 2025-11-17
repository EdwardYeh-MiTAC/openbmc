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
#include <mctp-packet.hpp>
#include <mctp.hpp>
#include <cstring>
#include <string>

namespace com::mitac_computing::mctp
{

MCTPPacketWrap::MCTPPacketWrap(const bool isMIC, const std::uint8_t msgType, const ssize_t reqSize)
    :isMIC(isMIC), msgType(msgType), reqData(nullptr), respData(nullptr), verboseLog(false)
{
    ssize_t fixedSize = reqSize;
    if (fixedSize < 2)
    {
        fixedSize = 10;
    }

    if (isMIC) {
        fixedSize += 4;
        this->reqData = (std::uint8_t*) malloc(fixedSize);
        this->msgType |= (isMIC ? 0x80 : 0x00);
    } else {
        this->reqData = (std::uint8_t*) malloc(fixedSize);
    }

    this->reqData[0] = this->msgType; //Intend to aligned with packet layout.
    this->reqSize = fixedSize;
}

MCTPPacketWrap::~MCTPPacketWrap() {
    if (reqData != nullptr)
        free(reqData);
    if (respData != nullptr)
        free(respData);
}

/*
MCTPCtrlMsgPacket::MCTPCtrlMsgPacket(const std::uint8_t cmdNum, const std::uint8_t instanceNum, const ssize_t reqSize)
    :MCTPPacketWrap(false, MCTP_MSG_TYPE_CONTROL, reqSize)
{
    std::cout << "MCTPCtrlMsgPacket: " << std::endl;
    if (this->reqData != nullptr) {
        this->reqData[1] = MCTP_CTRL_RQ | instanceNum;
        this->reqData[2] = cmdNum;
    }
}

MCTPNvmeMiMsgPacket::MCTPNvmeMiMsgPacket(const std::uint8_t cmdNum, const ssize_t reqSize)
    :MCTPPacketWrap(true, MCTP_MSG_TYPE_NVME, reqSize)
{
    std::cout << "MCTPNvmeMiMsgPacket: " << std::endl;
    if (this->reqData != nullptr) {
        this->reqData[1] = cmdNum;
    }
}
*/

};
