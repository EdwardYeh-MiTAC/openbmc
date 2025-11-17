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

#define MCTP_NET_ANY 0
#define MCTP_ADDR_ANY 0xff
#define MCTP_TAG_OWNER 0x08

#define MCTP_CTRL_RQ 0x80

// DSP0239 Table 1 – MCTP Message Types
#define MCTP_MSG_TYPE_CONTROL   0x00
#define MCTP_MSG_TYPE_PLDM      0x01
#define MCTP_MSG_TYPE_NCSI      0x02
#define MCTP_MSG_TYPE_ETH       0x03
#define MCTP_MSG_TYPE_NVME      0x04
#define MCTP_MSG_TYPE_SPDM      0x05
#define MCTP_MSG_TYPE_VD_PCI    0x7E
#define MCTP_MSG_TYPE_VD_IANA   0x7F

// DSP0239 Table 3 – MCTP physical transport binding identifiers
#define MCTP_TB_MCTP_OVER_SMBUS     0x01
#define MCTP_TB_MCTP_OVER_PCIE_VDM  0x02
#define MCTP_TB_MCTP_OVER_USB       0x03

enum MCTPTransportBindingType {
    SMBUS = 0x01,
    PCIE_VDM = 0x02,
    USB = 0x03
};

#define MCTP_CTRL_CMD_SET_ENDPOINT_ID   0x01
#define MCTP_CTRL_CMD_GET_ENDPOINT_ID   0x02
#define MCTP_CTRL_CMD_GET_ENDPOINT_UUID 0x03
#define MCTP_CTRL_CMD_GET_MCTP_VER      0x04
#define MCTP_CTRL_CMD_SET_GET_MSG_TYPE  0x05
#define MCTP_CTRL_CMD_SET_GET_VDM_TYPE  0x06
#define MCTP_CTRL_CMD_GET_ROUTING_TBL   0x0A

struct lladdrBDF {
    std::uint8_t devfunc;
    std::uint8_t bus;
};

typedef std::uint8_t mctp_eid_t;

struct mctp_addr {
        mctp_eid_t s_addr;
};

struct sockaddr_mctp {
        unsigned short int smctp_family;
        unsigned short __smctp_pad0;
        int smctp_network;
        struct mctp_addr smctp_addr;
        std::uint8_t smctp_type;
        std::uint8_t smctp_tag;
        std::uint8_t __smctp_pad1;
};

