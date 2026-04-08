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
#include <mctp.hpp>
#include <mctp-driver.hpp>
#include <mctp-packet.hpp>
#include <mutex>
#include <cstdint>
#include <map>
#include <vector>
#include <string>
#include <boost/asio.hpp>

namespace com::mitac_computing::mctp
{
class MCTPDriverHandle;
class MCTPEndPointBase;
class MCTPPacketWrap;

struct PCIePhyAddr
{
    std::uint8_t Bus;
    std::uint8_t Dev;
    std::uint8_t Func;
};

struct MCTPAddrInfo
{
    std::uint8_t net;
    mctp_eid_t eid;
};

uint32_t nvme_mi_crc32_update(uint32_t crc, void *data, size_t len);

class MCTPSocketUtils
{
    public:
        MCTPSocketUtils() = delete;
        static bool doSendReceive(const int socket, const struct MCTPAddrInfo destAddr, MCTPPacketWrap* dataPack);
};

class MCTPControlCmds
{
    public:
        static bool getVendorDefinedMessageSupport(const int socket, const struct MCTPAddrInfo destAddr, struct MCTPPacketWrap* dataPack);

};

class MCTPTransportBindingIntf
{
    public:
        MCTPTransportBindingIntf(enum MCTPTransportBindingType bindingType);
        ~MCTPTransportBindingIntf() = default;
        enum MCTPTransportBindingType getBindingType() const;
        bool bindMCTPEndPoint(MCTPEndPointBase* ep);
        virtual bool doSendReceive(const int socket, const struct MCTPAddrInfo destAddr, MCTPPacketWrap* dataPack) = 0;
    protected:
        const MCTPEndPointBase* getMCTPEndPoint();
    private:
        enum MCTPTransportBindingType _bindingType;
        MCTPEndPointBase* _ep;
};

class MCTPPCIeVDMBindingIntf: public MCTPTransportBindingIntf
{
    public:
        MCTPPCIeVDMBindingIntf(const struct PCIePhyAddr phyAddr);
        ~MCTPPCIeVDMBindingIntf();
        bool doSendReceive(const int socket, const struct MCTPAddrInfo destAddr, MCTPPacketWrap* dataPack);

        struct lladdrBDF getlladdrBDF();
    private:
        struct PCIePhyAddr _phyAddr;
};

struct PciVdmMsgTypeId {
    std::uint16_t PciVendorId;
    std::uint16_t VendorIdExt;
    bool operator==(const PciVdmMsgTypeId& other) const {
        return PciVendorId == other.PciVendorId && VendorIdExt == other.VendorIdExt;
    }
};

struct IanaVdmMsgTypeId {
    std::uint32_t Iana;
    std::uint16_t IanaExt;
    bool operator==(const IanaVdmMsgTypeId& other) const {
        return Iana == other.Iana && IanaExt == other.IanaExt;
    }
};

class MCTPEndPointBase
{
    public:
        MCTPEndPointBase(struct EPContext* epContext, const std::uint8_t net, const mctp_eid_t eid);
        ~MCTPEndPointBase();
        std::uint8_t getNet() const;
        std::uint8_t getEId() const;
        std::uint16_t getMTU() const;
        std::vector<struct PciVdmMsgTypeId> getPciVdmMsgTypes() const;

        bool startService();
        bool stopService();
        bool installDriverHandle(const MCTPDriverHandle* handle);

        bool setMCTPTransportBindingIntf(MCTPTransportBindingIntf* mctpTransBindingIntf);
        MCTPTransportBindingIntf* getMCTPTransportBindingIntf() const;

        bool doSendReceive(MCTPPacketWrap* dataPack) const;

        bool isMessageTypeSupported(const std::uint8_t typeId) const;
        bool isVDMTypeSupported(const struct PciVdmMsgTypeId typeId) const;
        bool isVDMTypeSupported(const struct IanaVdmMsgTypeId typeId) const;

        void dumpMsgTypes();
        void dumpPciVdmMsgTypes();
        void dumpIanaVdmMsgTypes();
        struct EPContext* getEPContext();

    protected:
        int getSocket() const;
        virtual bool startServiceImp() = 0;
        virtual bool stopServiceImp() = 0;
        struct EPContext* _epContext;
        std::uint8_t getInstanceId();
        bool isServiceRunning();

    private:
        std::uint8_t _Net;
        mctp_eid_t _EId;
        std::uint16_t _MTU;
        std::uint8_t _instanceId;
        std::mutex _iid_mtx;
        bool _serviceRunning;

        MCTPTransportBindingIntf* _mctpTransBindingIntf;
        boost::asio::steady_timer _DHTimer;    // Driver Handler
        void runDriverTask(boost::asio::steady_timer& timer);

        std::vector<std::uint8_t> _msgTypes;

        bool getMessageSupport();
        bool getVendorDefinedMessageSupport();
        std::vector<struct PciVdmMsgTypeId> _PciVdmMsgTypes;
        std::vector<struct IanaVdmMsgTypeId> _IanaVdmMsgTypes;
        
        std::mutex _MCTPDriverHandleMutex;
        std::map<std::string, const MCTPDriverHandle*> _mctpDriverHandles;
};

};
