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
#include <mutex>
#include <map>
#include <string>
#include <cstdint>
#include <mctp.hpp>
#include <boost/asio.hpp>
#include "mctp-packet.hpp"
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define IF_NAME_LENGTH 40

namespace com::mitac_computing::mctp
{

struct NetIntfInfo {
    std::string ifName;
    int ifIndex;
    bool isLinkUp;
    std::optional<uint32_t> net;
    std::optional<uint32_t> mtu;
};

class MctpNetlinkIntf {
    public:
        ~MctpNetlinkIntf();

        static MctpNetlinkIntf* getMctpNetlinkIntf(struct EPContext* epContext) {
            static MctpNetlinkIntf* gMctpNetlinkIntf = nullptr;
            if (gMctpNetlinkIntf == nullptr) {
                gMctpNetlinkIntf = new MctpNetlinkIntf(epContext);
            }
            return gMctpNetlinkIntf;
        };

        bool getIFIIndex(const std::string intfName, int& ifiIndex);
        int startService();
        int renewMCTPNetIntfList();
        int delRoute(const int ifindex, const mctp_eid_t eid);
        int addRoute(const int ifindex, const mctp_eid_t eid);
        int addNeigh(const int ifindex, const mctp_eid_t eid, const struct lladdrBDF lladdr);
        int delNeigh(const int ifindex, const mctp_eid_t eid);
        int getLink(const std::string ifName, struct NetIntfInfo& intfInfo);
    protected:
        int getSocket();
    private:
        MctpNetlinkIntf(struct EPContext* epContext);
        struct EPContext* _epContext;

        std::mutex _NetIntfInfoMutex;
        std::map<std::string, struct NetIntfInfo> _NetIntfInfoMap;
    
        int _sdNetlink; // Socket descriptor
        int initSocket();

        void updateNetlinkWdt(boost::asio::steady_timer& timer);
        boost::asio::steady_timer* _NLTimer;

        int sendReceive(struct nlmsghdr* req, struct nlmsghdr* resp);
        int sendReceive2(struct nlmsghdr* req, struct nlmsghdr** resp);

        struct rtattr* getRTAttr(int rta_type, struct rtattr *rta, size_t len, size_t *ret_len);
        size_t putRTAttr(struct rtattr **prta, size_t *rta_len, unsigned short type, const void *value, size_t val_len);
};

};
