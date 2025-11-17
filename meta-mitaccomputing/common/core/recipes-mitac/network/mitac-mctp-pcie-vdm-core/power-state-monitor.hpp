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
#include <vector>
#include <cstdint>
#include <boost/asio.hpp>
#include <thread>
#include <sdbusplus/asio/property.hpp>
#include <xyz/openbmc_project/State/Host/server.hpp>

namespace com::mitac_computing::utility
{
    using namespace sdbusplus::server::xyz::openbmc_project;
    enum class HostState
    {
        off,
        postInProgress,
        postComplete
    };

    class Observer {
        public:
            virtual bool update();
    };

    class Subject {
        public:
            bool attach(Observer* observer);
            bool detach(Observer* observer);
        protected:
            bool notify();
        private:
            std::vector<Observer*> _Observers;
    };

    class HostStateSubject: public Subject {
        public:
            HostStateSubject();
            ~HostStateSubject() = default;

            bool startService(boost::asio::io_context* io_context);
            bool updateSubject(HostState state);
            bool stopService();
            HostState getPreviousHostState();
            HostState getCurrentHostState();
        protected:
            boost::asio::io_context* _io_context;

            HostState _previousHostState;
            HostState _currentHostState;
            virtual bool startServiceImp(boost::asio::io_context* io_context);
            virtual bool stopServiceImp();
    };

    class HostStateObserver: public Observer {
        public:
            HostStateObserver(HostStateSubject* hostStateSubject);
            virtual ~HostStateObserver();
        protected:
            HostStateSubject* _hostStateSubject;
    };

    class Ast2600PEHRC4HostStateSubject: public HostStateSubject {
        public:
            Ast2600PEHRC4HostStateSubject();
            ~Ast2600PEHRC4HostStateSubject();

            virtual bool startServiceImp(boost::asio::io_context* io_context) override;
            virtual bool stopServiceImp() override;
            void pollingTask();
        private:
            bool running;
            std::thread* _iot;
            uint32_t _PEHRC4Addr = 0x1e6ed0c4;
    };
};
