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
#include "mctp-netlink.hpp"
#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <phosphor-logging/lg2.hpp>
#include <mutex>

std::mutex netlinkMutex;

namespace com::mitac_computing::mctp
{


MctpNetlinkIntf::MctpNetlinkIntf(struct EPContext* epContext)
    :_epContext(epContext), _sdNetlink(-1)
{
    initSocket();
}

MctpNetlinkIntf::~MctpNetlinkIntf() {
    if (_NLTimer) {
        delete _NLTimer;
    }

    if (_sdNetlink > 0) {
        close(_sdNetlink);
    }
}

int MctpNetlinkIntf::initSocket() {
    struct sockaddr_nl  local;
    memset(&local, 0, sizeof(local));

    local.nl_family = AF_NETLINK;
    local.nl_pid = 0;

    _sdNetlink = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   // create netlink socket

    if (_sdNetlink < 0) {
        std::cerr << "Failed to create netlink socket" << std::endl;
        return -1;
    } else {
        if (bind(_sdNetlink, (struct sockaddr*)&local, sizeof(local)) < 0) {     // bind socket
            std::cerr << "Failed to bind netlink socket" << std::endl;
            close(_sdNetlink);
            return -1;
        }
        int opt = 1;
        setsockopt(_sdNetlink, SOL_NETLINK, NETLINK_GET_STRICT_CHK, &opt, sizeof(opt));
        setsockopt(_sdNetlink, SOL_NETLINK, NETLINK_EXT_ACK, &opt, sizeof(opt));
    }
    return 0;
};

int MctpNetlinkIntf::getSocket() {
    return _sdNetlink;
}

void MctpNetlinkIntf::updateNetlinkWdt(boost::asio::steady_timer& timer) {
    std::cout << "MctpNetlinkIntf::updateNetlinkWdt" << std::endl;
    // TODO:
    timer.expires_after(std::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code& error) {
        if (error != boost::asio::error::operation_aborted) {
            this->updateNetlinkWdt(timer);
        }
    });
}

int MctpNetlinkIntf::startService() {
    /*
    std::cout << "MctpNetlinkIntf::startService cp1" << std::endl;
    if (_epContext == nullptr)
        return -1;
    std::cout << "MctpNetlinkIntf::startService cp2" << std::endl;
    _NLTimer = new boost::asio::steady_timer(_epContext->io, std::chrono::seconds(1));
    if (_NLTimer == nullptr)
        return -1;

    std::cout << "MctpNetlinkIntf::startService cp3" << std::endl;
    _NLTimer->async_wait([&](const boost::system::error_code& error) {
        this->updateNetlinkWdt(*_NLTimer);
    });
    std::cout << "MctpNetlinkIntf::startService cp4" << std::endl;
    */
    return 0;
}

int MctpNetlinkIntf::sendReceive(struct nlmsghdr* req, struct nlmsghdr* resp) {
    socklen_t addrlen;
    struct sockaddr_nl local;
    int ifindex = 2;
    size_t len;
    int rc;

    std::lock_guard<std::mutex> lock(netlinkMutex);

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = 0;


    std::cout << "Req: " << std::endl << "  ";
    const std::uint8_t* u8Req = (std::uint8_t*) req;
    for (ssize_t offset = 0; offset < req->nlmsg_len; offset ++) {
        if ((offset != 0) && (offset % 16 == 0))
            std::cout << std::endl << "  ";
        std::stringstream stream;
        stream << "0x" << std::hex << std::setfill('0') << std::setw(2) <<  (uint16_t) u8Req[offset] << " ";
        std::cout << stream.str();
    }
    std::cout << std::endl;

    rc = sendto(this->getSocket(), req, req->nlmsg_len, 0,
                (struct sockaddr *)&local, sizeof(local));
    if (rc < 0) {
        std::cerr << "get_neigh rc < 0" << std::endl;
        return rc;
    }

    if (rc != (int)req->nlmsg_len) {
        std::cerr << "short send" << std::endl;
        return rc;
    }

    if (req->nlmsg_flags & NLM_F_ACK) {
        struct {
            struct nlmsghdr hdr;
            std::uint8_t buff[1000];
        } resp;

        recvfrom(this->getSocket(), &resp, sizeof(&resp), 0, NULL, NULL);
        if (rc < 0) {
            return rc;
        }

        struct nlmsghdr* hdr = &resp.hdr;
        for (; NLMSG_OK(hdr, len); hdr = NLMSG_NEXT(hdr, len)) {
            if (hdr->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr* errmsg = (struct nlmsgerr*) NLMSG_DATA(hdr);
                std::cerr << "Got error" << std::endl;
                return errmsg->error;
            } else {
                std::cerr << "Unknown type" << std::endl;
            }
        }
        std::cout << "Acked" << std::endl;
        return 0;
    }
    return 0;
}

size_t MctpNetlinkIntf::putRTAttr(struct rtattr **prta, size_t *rta_len,
                 unsigned short type, const void *value,
                 size_t val_len)
{
    struct rtattr *rta = *prta;
    rta->rta_type = type;
    rta->rta_len = RTA_LENGTH(val_len);
    memcpy(RTA_DATA(rta), value, val_len);
    *prta = RTA_NEXT(*prta, *rta_len);
    return RTA_SPACE(val_len);
}

struct rtattr* MctpNetlinkIntf::getRTAttr(int rta_type, struct rtattr *rta, size_t len,
                size_t *ret_len)
{
    for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta->rta_type == rta_type) {
            if (ret_len) {
                *ret_len = RTA_PAYLOAD(rta);
            }
            return (struct rtattr*) RTA_DATA(rta);
        }
    }
    if (ret_len) {
        *ret_len = 0;
    }
    return NULL;
}

int MctpNetlinkIntf::sendReceive2(struct nlmsghdr* req, struct nlmsghdr** resp) {
    int rc;
    struct sockaddr_nl local;
    socklen_t addrlen;
    addrlen = sizeof(local);
    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = 0;

    std::lock_guard<std::mutex> lock(netlinkMutex);

    rc = sendto( this->getSocket(), req, req->nlmsg_len, 0,
                (struct sockaddr *)&local, addrlen);
    if (rc < 0)
        return rc;

    if (rc != (int)req->nlmsg_len) {
        std::cerr << "short send" << std::endl;
        return rc;
    }

    void* respbuf = nullptr;
    size_t readlen = 0;
    size_t newlen = 0;
    size_t pos = 0;

    bool done = false;
    while (!done) {
        rc = recvfrom( this->getSocket(), NULL, 0, MSG_TRUNC | MSG_PEEK,
                      NULL, NULL);
        if (rc < 0) {
            break;
        }
        if (!rc)
            break;

        readlen = rc;
        newlen = pos + readlen;
        respbuf = realloc(respbuf, newlen);
        if (!respbuf) {
            std::cerr << "failed to reallocate respbuf" << std::endl;
        }
        void* resp = respbuf + pos;

        rc = recvfrom( this->getSocket(), resp, readlen, MSG_TRUNC,
                      (struct sockaddr *)&local, &addrlen);
        if (rc < 0) {
            std::cerr << "recvfrom failed." << std::endl;
            break;
        }
        if (rc != readlen) {
            std::cerr << "Invalid return len" << std::endl;
            break;
        }

        struct nlmsghdr* nlResp = (struct nlmsghdr*) resp;

        /*
        std::cout << "Resp: " << std::endl << "  ";
        const std::uint8_t* u8Resp = (std::uint8_t*) resp;
        for (ssize_t offset = 0; offset < readlen; offset++) {
            if ((offset != 0) && (offset % 16 == 0))
                std::cout << std::endl << "  ";
            std::stringstream stream;
            stream << "0x" << std::hex << std::setfill('0') << std::setw(2) <<  (uint16_t) u8Resp[offset] << " ";
            std::cout << stream.str();
        }
        std::cout << std::endl;
        */

        for (; NLMSG_OK(nlResp, readlen); nlResp = NLMSG_NEXT(nlResp, readlen)) {
            if (done)
                break;
            done = (nlResp->nlmsg_type == NLMSG_DONE) ||
                    !(nlResp->nlmsg_flags & NLM_F_MULTI);
        }
        pos = (newlen < pos + rc) ? newlen : pos + rc;
    }
    *resp = (struct nlmsghdr*) respbuf;
    return newlen;
}

int MctpNetlinkIntf::renewMCTPNetIntfList() {
    /*
    std::map<std::string, struct NetIntfInfo> netIntfInfoMap;
    struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifmsg;
    } msg = { 0 };

    struct sockaddr_nl local;
    socklen_t addrlen;
    size_t buflen;
    void *buf;
    addrlen = sizeof(local);

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = 0;

    msg.nh.nlmsg_len = NLMSG_LENGTH(sizeof(msg.ifmsg));
    msg.nh.nlmsg_type = RTM_GETLINK;
    msg.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    msg.ifmsg.ifi_family = AF_MCTP;

    struct nlmsghdr* response = nullptr;
    int rc = this->sendReceive2((struct nlmsghdr*) &msg, &response);
    struct ifinfomsg* info;
    if ((rc > 0) && (response != nullptr)) {
        std::cout << "sendReceive2 return valid response " << std::endl;

        for (; NLMSG_OK(response, rc); response = NLMSG_NEXT(response, rc)) {
            if (response->nlmsg_type == NLMSG_DONE) {
                break;
            }

            if (response->nlmsg_type == NLMSG_ERROR) {
                break;
            }

            struct rtattr *rta = NULL;
            if (NLMSG_PAYLOAD(response, 0) < sizeof(*info)) {
                std::cout << "sendReceive2 contains invalid payload. " << std::endl;
                return -1;
            }
            info = (struct ifinfomsg*) NLMSG_DATA(response);
            if (!info->ifi_index)
                continue;

            size_t rlen, nlen, mlen;
            struct rtattr *rt_mctp = NULL;
            rta = (struct rtattr*)(info + 1);
            rlen = NLMSG_PAYLOAD(response, sizeof(*info));
            struct rtattr* rt_nest = this->getRTAttr(IFLA_AF_SPEC, rta, rlen, &nlen);
            if (rt_nest) {
                rt_mctp = this->getRTAttr(AF_MCTP, rt_nest, nlen,
                            &mlen);
            }
            if (!rt_mctp) {
                continue;
            }
            size_t ifname_len;
            const char* ifname = (const char*) this->getRTAttr(IFLA_IFNAME, rta, rlen,
                           &ifname_len);
            if (!ifname) {
                std::cerr << "No IFLA_IFNAME" << std::endl;
                continue;
            }
            ifname_len = strnlen(ifname, ifname_len);
            struct NetIntfInfo niInfo;
            niInfo.ifName = ifname;
            niInfo.ifIndex = info->ifi_index;
            netIntfInfoMap.insert(std::pair<std::string, struct NetIntfInfo>(ifname, niInfo));
        }
        std::cout << "Updating _NetIntfInfoMap" << std::endl;
        std::lock_guard<std::mutex> guard(_NetIntfInfoMutex);
        _NetIntfInfoMap = netIntfInfoMap;
        for (const auto& pair : _NetIntfInfoMap)
            std::cout << "Interface[ " << pair.second.ifIndex << " ] = " << pair.first << std::endl;
        return 0;
    }
    std::cerr << "sendReceive2 return failed. rc = " << rc << std::endl;
    return -1;
*/
    return 0;
}

int MctpNetlinkIntf::addRoute(const int ifindex, const mctp_eid_t eid) {
    struct {
        struct nlmsghdr nh;
        struct rtmsg rtmsg;
        std::uint8_t rta_buff[50];
    } addNewRouteReq;
    struct rtattr *rta;
    size_t rta_len;
    // std::cout << "Size of ndmsg: " << sizeof(struct ndmsg) << std::endl;

    memset(&addNewRouteReq, 0x0, sizeof(addNewRouteReq));
    addNewRouteReq.nh.nlmsg_type = RTM_NEWROUTE;
    addNewRouteReq.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    addNewRouteReq.rtmsg.rtm_family = AF_MCTP;
    addNewRouteReq.rtmsg.rtm_type = RTN_UNICAST;
    addNewRouteReq.nh.nlmsg_len = NLMSG_LENGTH(sizeof(addNewRouteReq.rtmsg));
    rta_len = sizeof(addNewRouteReq.rta_buff);
    rta = (struct rtattr*) addNewRouteReq.rta_buff;
    addNewRouteReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, RTA_DST,
                           &eid, sizeof(eid));
    addNewRouteReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, RTA_OIF,
                           &ifindex, sizeof(ifindex));

    struct nlmsghdr* resp = nullptr;
    int rc = this->sendReceive2(&addNewRouteReq.nh, &resp);
    if (resp != nullptr)
        free(resp);
    return rc;
}

int MctpNetlinkIntf::delRoute(const int ifindex, const mctp_eid_t eid) {
    struct {
        struct nlmsghdr nh;
        struct rtmsg rtmsg;
        std::uint8_t rta_buff[50];
    } addDelRouteReq;
    struct rtattr *rta;
    size_t rta_len;
    // std::cout << "Size of ndmsg: " << sizeof(struct ndmsg) << std::endl;

    memset(&addDelRouteReq, 0x0, sizeof(addDelRouteReq));
    addDelRouteReq.nh.nlmsg_type = RTM_DELROUTE;
    addDelRouteReq.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    addDelRouteReq.rtmsg.rtm_family = AF_MCTP;
    addDelRouteReq.rtmsg.rtm_type = RTN_UNICAST;
    addDelRouteReq.nh.nlmsg_len = NLMSG_LENGTH(sizeof(addDelRouteReq.rtmsg));
    rta_len = sizeof(addDelRouteReq.rta_buff);
    rta = (struct rtattr*) addDelRouteReq.rta_buff;
    addDelRouteReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, RTA_DST,
                           &eid, sizeof(eid));
    addDelRouteReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, RTA_OIF,
                           &ifindex, sizeof(ifindex));

    struct nlmsghdr* resp = nullptr;
    int rc = this->sendReceive2(&addDelRouteReq.nh, &resp);
    if (resp != nullptr)
        free(resp);
    return rc;
}

int MctpNetlinkIntf::addNeigh( const int ifindex, const mctp_eid_t eid, const struct lladdrBDF lladdr) {
    struct {
        struct nlmsghdr nh;
        struct ndmsg ndmsg;
        std::uint8_t rta_buff[50];
    } addNewNeighReq;
    struct rtattr *rta;
    size_t rta_len;

    memset(&addNewNeighReq, 0x0, sizeof(addNewNeighReq));
    addNewNeighReq.nh.nlmsg_type = RTM_NEWNEIGH;
    addNewNeighReq.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    addNewNeighReq.ndmsg.ndm_ifindex = ifindex;
    addNewNeighReq.ndmsg.ndm_family = AF_MCTP;
    addNewNeighReq.nh.nlmsg_len = NLMSG_LENGTH(sizeof(addNewNeighReq.ndmsg));
    rta_len = sizeof(addNewNeighReq.rta_buff);
    rta = (struct rtattr*) addNewNeighReq.rta_buff;
    addNewNeighReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, NDA_DST,
                           &eid, sizeof(eid));
    addNewNeighReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, NDA_LLADDR,
                           &lladdr, sizeof(lladdr));

    struct nlmsghdr* resp = nullptr;
    int rc = this->sendReceive2(&addNewNeighReq.nh, &resp);
    if (resp != nullptr)
        free(resp);

    return 0;
}

int MctpNetlinkIntf::delNeigh( const int ifindex, const mctp_eid_t eid) {
    struct {
        struct nlmsghdr nh;
        struct ndmsg ndmsg;
        std::uint8_t rta_buff[50];
    } addNewNeighReq;
    struct rtattr *rta;
    size_t rta_len;

    memset(&addNewNeighReq, 0x0, sizeof(addNewNeighReq));
    addNewNeighReq.nh.nlmsg_type = RTM_DELNEIGH;
    addNewNeighReq.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    addNewNeighReq.nh.nlmsg_len = NLMSG_LENGTH(sizeof(addNewNeighReq.ndmsg));

    addNewNeighReq.ndmsg.ndm_ifindex = ifindex;
    addNewNeighReq.ndmsg.ndm_family = AF_MCTP;

    rta_len = sizeof(addNewNeighReq.rta_buff);
    rta = (struct rtattr*) addNewNeighReq.rta_buff;
    addNewNeighReq.nh.nlmsg_len += this->putRTAttr(&rta, &rta_len, NDA_DST,
                           &eid, sizeof(eid));
    struct nlmsghdr* resp = nullptr;
    int rc = this->sendReceive2(&addNewNeighReq.nh, &resp);
    if (resp != nullptr)
        free(resp);

    return rc;
}

int MctpNetlinkIntf::getLink(const std::string targetIfName, struct NetIntfInfo& intfInfo) {
	struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifmsg;
    } getIfInfoMsg = { 0 };

    getIfInfoMsg.nh.nlmsg_len = NLMSG_LENGTH(sizeof(getIfInfoMsg.ifmsg));
    getIfInfoMsg.nh.nlmsg_type = RTM_GETLINK;
    getIfInfoMsg.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    getIfInfoMsg.ifmsg.ifi_family = AF_MCTP;

    struct nlmsghdr* resp = nullptr;
    int rc = this->sendReceive2(&getIfInfoMsg.nh, &resp);
    if ((rc < 0) || (resp == nullptr)) {
        return -1;
    }

    struct ifinfomsg *info;
    struct nlmsghdr* nlh = resp;
	bool done = false;
    for (; NLMSG_OK(nlh, rc); nlh = NLMSG_NEXT(nlh, rc)) {
        if (nlh->nlmsg_type == NLMSG_DONE) {
            done = true;
            break;
        }

        if (nlh->nlmsg_type == NLMSG_ERROR) {
            done = true;
            break;
        }

        struct rtattr *rta = NULL;
        if (NLMSG_PAYLOAD(nlh, 0) < sizeof(*info))
            return -1;

        info = (struct ifinfomsg*) NLMSG_DATA(nlh);
        if (!info->ifi_index)
            continue;

        size_t rlen, nlen, mlen;
        struct rtattr *rt_mctp = NULL;
        rta = (struct rtattr*)(info + 1);
        rlen = NLMSG_PAYLOAD(nlh, sizeof(*info));
        size_t respLen;
        const char* ifname = (const char*) this->getRTAttr(IFLA_IFNAME, rta, rlen,
                       &respLen);

        if (targetIfName != ifname) {
            lg2::debug("Mismatch inname. Found: {NAME}.", "NAME", ifname);
            continue;
        }
        intfInfo.ifName = ifname;
        intfInfo.ifIndex = info->ifi_index;

        std::uint32_t* mtu = (std::uint32_t*) this->getRTAttr(IFLA_MTU, rta, rlen, &respLen);
        if ((mtu != nullptr) && (respLen == sizeof(mtu))) {
            intfInfo.mtu = std::make_optional<std::uint32_t>(*mtu);
            lg2::debug("Found IFLA_MTU for: {NAME}.", "NAME", ifname);
        }

        struct rtattr* rt_nest = this->getRTAttr(IFLA_AF_SPEC, rta, rlen, &nlen);
        if (rt_nest) {
            rt_mctp = this->getRTAttr(AF_MCTP, rt_nest, nlen,
                        &mlen);
        }
        if (!rt_mctp) {
            /* Skip non-MCTP interfaces */
            continue;
        }

        lg2::debug("Found AF_MCTP from IFLA_AF_SPEC for for: {NAME}.", "NAME", ifname);
        size_t attLen;
        std::uint32_t* net = (std::uint32_t*) this->getRTAttr(IFLA_MCTP_NET, rt_mctp, mlen, &attLen);
        if ((net != nullptr) && (attLen == sizeof(net))) {
            intfInfo.net = std::make_optional<std::uint32_t>(*net);
            lg2::debug("Found IFLA_MCTP_NET for: {NAME}.", "NAME", ifname);
        }
        return 0;
    }
    free(resp);  
    return -1; 
}


}
