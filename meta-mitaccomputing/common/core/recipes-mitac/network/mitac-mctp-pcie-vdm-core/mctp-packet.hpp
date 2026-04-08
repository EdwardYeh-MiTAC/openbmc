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
#include <cstdint>
#include <iostream>
#include <mctp.hpp>
#include <boost/asio.hpp>

namespace com::mitac_computing::mctp
{
struct EPContext {
    int socketDesc;
    boost::asio::io_context io;
};

class MCTPPacketWrap {
    public:
        MCTPPacketWrap(const bool isMIC, const std::uint8_t msgType, const ssize_t reqSize);
        ~MCTPPacketWrap();
        bool verboseLog;
        bool isMIC;
        std::uint8_t msgType;
        std::uint8_t* reqData;
        ssize_t reqSize;
        std::uint8_t* respData;
        ssize_t respSize;
};

template <typename T>
class MCTPPacket: public MCTPPacketWrap {
    public:
        MCTPPacket(const bool isMIC, const std::uint8_t msgType):MCTPPacketWrap(isMIC, msgType, sizeof(T)) {};
        MCTPPacket(const bool isMIC, const std::uint8_t msgType, const std::uint8_t instanceNum):MCTPPacketWrap(isMIC, msgType, sizeof(T)) {
            switch (msgType)
            {
                case MCTP_MSG_TYPE_CONTROL:
                    {
                        if (this->reqData != nullptr) {
                            this->reqData[1] = MCTP_CTRL_RQ | instanceNum;
                        }
                        break;
                    }
                default:
                    std::cerr << "This constructor is for request needs to have instance number for messages." << std::endl;
            };
        };
        ~MCTPPacket() = default;
        T* getReqData() {return (T*) this->reqData;};
};

#pragma pack(push, 1)
struct mctp_ctrl_generic_cmd {
    std::uint8_t type;
    std::uint8_t instanceNum;
    std::uint8_t subCmc;
};

struct mctp_ctrl_handle_cmd {
    std::uint8_t type;
    std::uint8_t instanceNum;
    std::uint8_t subCmc;
    std::uint8_t handle;
};

struct mctp_ctrl_set_endpoint_id {
    std::uint8_t type;
    std::uint8_t instanceNum;
    std::uint8_t subCmc;
    std::uint8_t op;
    std::uint8_t eid;
};

struct mctp_ctrl_allocate_eids {
    std::uint8_t type;
    std::uint8_t instanceNum;
    std::uint8_t subCmc;
    std::uint8_t op;
    std::uint8_t eidCount;
    std::uint8_t startEId;
};

struct nvme_mi_req {
    std::uint8_t type;
    std::uint8_t cmd;
    std::uint8_t rsvd1;
    std::uint8_t rsvd2;
    std::uint8_t opcode;
    std::uint8_t rsvd3;
    std::uint8_t rsvd4;
    std::uint8_t rsvd5;
    std::uint8_t dw0_1;
    std::uint8_t dw0_2;
    std::uint8_t dw0_3;
    std::uint8_t dw0_4;
    std::uint8_t dw1_1;
    std::uint8_t dw1_2;
    std::uint8_t dw1_3;
    std::uint8_t dw1_4;
};
#pragma pack(pop)   

};
