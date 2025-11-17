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
#include "power-state-monitor.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/State/OperatingSystem/Status/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/Asset/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/UniqueIdentifier/server.hpp>

#define RENEW_MRD_WDT_TIMEOUT 120

namespace com::mitac_computing::utility
{
using namespace sdbusplus::server::xyz::openbmc_project;
using HostStateHandler = std::function<void(HostState, HostState)>;
using PowerState = state::Host::HostState;
using OsState = state::operating_system::Status::OSStatus;

HostState hostState = HostState::off;
static PowerState powerState = PowerState::Off;
static OsState osState = OsState::Inactive;
static bool biosDone = false;



bool Observer::update() {
    return true;
}

bool Subject::attach(Observer* observer) {
    if (observer == nullptr)
        return false;

    if (std::find(_Observers.begin(), _Observers.end(), observer) != _Observers.end())
        return true;

    _Observers.push_back(observer);
    return true;
}

bool Subject::detach(Observer* observer) {
    if (observer == nullptr)
        return false;

    _Observers.erase(std::find(_Observers.begin(), _Observers.end(), observer));
    return true;
}

bool Subject::notify() {
    for (Observer* observer : _Observers) {
        if (observer)
            observer->update();
    }
    return true;
}

HostStateSubject::HostStateSubject()
    :_currentHostState(HostState::off), _previousHostState(HostState::off), _io_context(nullptr) {

}

HostState HostStateSubject::getPreviousHostState() {
    return _previousHostState;
}

HostState HostStateSubject::getCurrentHostState() {
    return _currentHostState;
}

bool HostStateSubject::updateSubject(HostState state) {
    _previousHostState = _currentHostState;
    _currentHostState = state;
    this->notify();
    return true;
}

bool HostStateSubject::startService(boost::asio::io_context* io_context) {
    if (io_context == nullptr)
        return false;
    _io_context = io_context;

    return startServiceImp(io_context);
}

bool HostStateSubject::stopService() {
    return stopServiceImp();
}

bool HostStateSubject::startServiceImp(boost::asio::io_context* io_context) {
    if (io_context == nullptr)
        return false;
    return true;
}

bool HostStateSubject::stopServiceImp() {
    return true;
}


HostStateObserver::HostStateObserver(HostStateSubject* hostStateSubject)
    :_hostStateSubject(hostStateSubject)
{

}

HostStateObserver::~HostStateObserver() {
    if (_hostStateSubject) {
        _hostStateSubject->detach(this);
    }
}

Ast2600PEHRC4HostStateSubject::Ast2600PEHRC4HostStateSubject()
    :running(false)
{
    _PEHRC4Addr = 0x1e6ed0c4;
}

Ast2600PEHRC4HostStateSubject::~Ast2600PEHRC4HostStateSubject()
{
    if (_iot) {
        this->stopService();
        delete _iot;
    }
}

void Ast2600PEHRC4HostStateSubject::pollingTask() {
    unsigned page_size, mapped_size, offset_in_page;
    unsigned width = 32;
    uint32_t previous_reading = 0;
    uint32_t current_reading = 0;
    uint32_t physical_address = _PEHRC4Addr;


    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd == -1) {
        lg2::error("Ast2600PEHRC4HostStateSubject::pollingTask unable to open memory device");
        return;
    }
    mapped_size = page_size = getpagesize();
    offset_in_page = (unsigned)physical_address & (page_size - 1);
    if (offset_in_page + width > page_size) {
        mapped_size *= 2;
    }
    void* mapped_address = mmap(NULL, mapped_size, PROT_READ, MAP_SHARED, fd, physical_address & ~(uint32_t)(page_size - 1));
    if (mapped_address == MAP_FAILED) {
        lg2::error("Ast2600PEHRC4HostStateSubject::pollingTask unable to map memory");
        close(fd);
        return;
    }

    this->running = true;
    while (this->running) {
        volatile uint32_t* data_ptr = static_cast<volatile uint32_t*>(mapped_address + offset_in_page);
        previous_reading = current_reading;
        current_reading = *data_ptr;

        if (current_reading != previous_reading) {
            uint8_t PERST = (current_reading >> 19) & 0x01;
            uint8_t BusNum = (current_reading >> 5) & 0xFF;
            uint8_t DevNum = (current_reading >> 0) & 0x1F;

            _previousHostState = _currentHostState;
            if (PERST == 0) {
                lg2::info("Ast2600PEHRC4HostStateSubject::pollingTask Notify HostState::off");
                this->updateSubject(HostState::off);
            } else {
                if ((BusNum == 0) && (DevNum ==0)) {
                    lg2::info("Ast2600PEHRC4HostStateSubject::pollingTask Notify HostState::postInProgress");
                    this->updateSubject(HostState::postInProgress);
                } else {
                    lg2::info("Ast2600PEHRC4HostStateSubject::pollingTask Notify HostState::postComplete");
                    this->updateSubject(HostState::postComplete);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if (munmap(mapped_address, mapped_size) == -1)
        lg2::error("Ast2600PEHRC4HostStateSubject::pollingTask unable to umap memory");

    if (fd >= 0)
        close(fd);
}

bool Ast2600PEHRC4HostStateSubject::startServiceImp(boost::asio::io_context* io_context) {
    if (this->running) {
        return true;
    }

    _iot = new std::thread(boost::bind(&Ast2600PEHRC4HostStateSubject::pollingTask, this));
    if (_iot)
        return true;
    else
        return false;
}

bool Ast2600PEHRC4HostStateSubject::stopServiceImp() {
    lg2::debug("Ast2600PEHRC4HostStateSubject::stopServiceImp()");
    this->running = false;
    _iot->join(); 
    lg2::debug("Ast2600PEHRC4HostStateSubject::stopServiceImp() done");
    return true;
}

}
