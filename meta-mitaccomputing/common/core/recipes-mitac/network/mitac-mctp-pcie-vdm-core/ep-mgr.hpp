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
#include "mctp-endpoint-base.hpp"
#include "mctp-netlink.hpp"
#include "mctp-driver-mgr.hpp"
#include "mctp-driver.hpp"
#include "power-state-monitor.hpp"
#include <cstdint>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <thread>
#include <vector>

namespace com::mitac_computing::mctp
{
class EndpointManager;
//https://segmentfault.com/a/1190000006691692
class ThreadPool
{
    public:
        explicit ThreadPool(std::size_t size)
            : work_guard_(boost::asio::make_work_guard(io_context_)) {
                workers_.reserve(size);
                for (std::size_t i = 0; i < size; ++i) {
                    workers_.emplace_back(&boost::asio::io_context::run, &io_context_);
                }
            }

        ~ThreadPool() {
            io_context_.stop();
            for (auto& w : workers_) {
                w.join();
            }
        }

        template<class F>
        void Enqueue(F f) {
            boost::asio::post(io_context_, f);
        }

    private:
        std::vector<std::thread> workers_;
        boost::asio::io_context io_context_;

        typedef boost::asio::io_context::executor_type ExecutorType;
        boost::asio::executor_work_guard<ExecutorType> work_guard_;
};

class MCTPEPHostStateObserver;
class MCTPEndPoint: public MCTPEndPointBase
{
    public:
        MCTPEndPoint(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid);
        ~MCTPEndPoint() = default;

        virtual void notifyHostStateChange(com::mitac_computing::utility::HostState newHostState);
        virtual void notifySystemReset();
        virtual bool createHostStateObserver(com::mitac_computing::utility::HostStateSubject* pwrStateSubject);
    protected:
        virtual bool startServiceImp();
        virtual bool stopServiceImp();

    private:
        MCTPEPHostStateObserver* _EPHostStateObserver;
};

class MCTPEPHostStateObserver: public com::mitac_computing::utility::HostStateObserver
{
    public:
        MCTPEPHostStateObserver(MCTPEndPoint* ep, com::mitac_computing::utility::HostStateSubject* pwrStateSubject);
        ~MCTPEPHostStateObserver() = default;
        virtual bool update();

    private:
        MCTPEndPoint* _ep;
};

enum RoutingEntryStatus {
    Deleted = -1,
    Unchanged = 0,
    Added = 1
};

struct RoutingTableConfig {
    std::uint8_t startEId;
    std::uint8_t EIdCount;
};


class MCTPBridge: public MCTPEndPoint
{
    public:
        MCTPBridge(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid);
        MCTPBridge(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid);
        ~MCTPBridge();
        void bindMctpNetlinkIntf(MctpNetlinkIntf* mctpNLIntf);
        void bindEndpointManager(EndpointManager* epmgr);
        EndpointManager* getEndpointManager();
        void updateRoutingTableWdt(boost::asio::steady_timer& timer);
        void setPollingHostState(com::mitac_computing::utility::HostState newPollingHostState);

        virtual void notifyHostStateChange(com::mitac_computing::utility::HostState newHostState);
        virtual void notifySystemReset();
        virtual bool createHostStateObserver(com::mitac_computing::utility::HostStateSubject* pwrStateSubject);

        // setRoutingTableConfig, getRoutingTableConfig only valid when BMC is BusOwner.
        bool setRoutingTableConfig(const struct RoutingTableConfig& routingTableConfig);
        struct RoutingTableConfig*  getRoutingTableConfig();
    protected:
        virtual bool startServiceImp();
        virtual bool stopServiceImp();
        bool startRoutingTableService();
        bool stopRoutingTableService();

        MctpNetlinkIntf* getMctpNetlinkIntf();

        struct NetIntfInfo _netIntfInfo;

        int addEndpoint(const std::uint8_t eid, const struct PCIePhyAddr addr);
        int removeEndpoint(const std::uint8_t eid, const struct PCIePhyAddr addr);

    private:
        //bool getRoutingTableEntry(const std::uint8_t entryHandle, const std::uint8_t* respBuffer, ssize_t& respSize);
        std::vector<std::uint8_t> _rtRawData;
        std::uint32_t _rtCRC32;

        struct RoutingTableConfig* _routingTableConfig;
        void dumpRoutingTable(std::vector<std::uint8_t>& table);
        void handleRoutingTableUpdate(std::vector<std::uint8_t>& oldTable, std::vector<std::uint8_t>& newTable);
        boost::asio::steady_timer _RTTimer;

        MctpNetlinkIntf* _mctpNLIntf;
        EndpointManager* _epmgr;

        std::mutex _EPMutex;
        std::vector<MCTPEndPoint*> _endpoints;

        com::mitac_computing::utility::HostStateSubject* _hostStateSubject;
        com::mitac_computing::utility::HostState _pollingHostState;
};

class MCTPBusOwner: public MCTPBridge
{
    public:
        MCTPBusOwner(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid);
        MCTPBusOwner(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid);
        ~MCTPBusOwner() = default;
    protected:

    private:
        //virtual bool startServiceImp();
};

class MCTPBusBridgeAtRootComplex: public MCTPBridge
{
    public:
        MCTPBusBridgeAtRootComplex(struct EPContext* epContext, const std::uint8_t net, const std::uint8_t eid);
        MCTPBusBridgeAtRootComplex(struct EPContext* epContext, const struct NetIntfInfo netIntfInfo, const std::uint8_t eid);
        ~MCTPBusBridgeAtRootComplex() = default;
    protected:
        virtual bool startServiceImp() override;
        virtual bool stopServiceImp() override;

    private:
        bool initBusBridge();
        bool sendPrepareEndpointDiscovery();
        bool sendEndpointDiscovery();
        bool sendSetEndpointID();
        bool sendAllocateEndpointIDs();

        void RTInitWdt(boost::asio::steady_timer& timer);
        boost::asio::steady_timer _RTInitTimer;
};

class EndpointManager
{
    public:
        EndpointManager(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr);
        ~EndpointManager();

        bool startService();
        std::uint16_t getMTU();

        struct EPContext* getEPContext();

        int addEndpoint(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr);
        int removeEndpoint(std::uint8_t net, std::uint8_t eid, struct PCIePhyAddr addr);

        int loadConfig(std::string fn);

        int bindDriver(MCTPEndPoint* ep);
    private:
        std::uint8_t _Net;
        std::uint8_t _BO_EID;

        struct EPContext _epContext;

        int _SocketDesc;
        boost::asio::io_context io_context;

        struct PCIePhyAddr _BO_Addr;

        bool _hasInited;
        std::uint8_t _instanceId;

        std::uint16_t _MTU;

        int getSocket();
        std::uint8_t getInstanceId();

        std::mutex _EPMutex;
        std::vector<MCTPEndPoint*> _endpoints;

        MctpNetlinkIntf* _mctpNetlinkIntf;

        MCTPDriverMgr mctpDrvMgr;
        void loadDrivers();
        void loadDrivers(const nlohmann::json& jsonObj);

        void attachBusOwnerEndpoint(const nlohmann::json& jsonObj);
        void attachPCIeBusOwnerEndpoint(const std::string moduleName, const std::string intfName, const mctp_eid_t eid, const struct PCIePhyAddr phyAddr);
        void attachBusBridgeEndpoint(const nlohmann::json& jsonObj);
        void attachPCIeBusBridgeEndpoint(const std::string moduleName, const std::string intfName, const mctp_eid_t eid, const struct PCIePhyAddr phyAddr, const struct RoutingTableConfig routingTableConfig);
        std::vector<MCTPBusOwner*> _BusOwners;
        std::vector<MCTPBridge*> _BusBridges;
        //void attachBusBridgeEndpoint(const nlohmann::json& jsonObj);
        com::mitac_computing::utility::HostStateSubject* _hostStateSubject;
        //void bindDrivers();

        //void startEPService();
};

};
