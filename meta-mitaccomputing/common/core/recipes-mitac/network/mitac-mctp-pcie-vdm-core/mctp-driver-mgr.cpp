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
#include "mctp-driver-mgr.hpp"

#include <phosphor-logging/lg2.hpp>
#include <iostream>
#include <dlfcn.h>

using namespace com::mitac_computing::mctp;

MCTPDriverMgr::MCTPDriverMgr() {

}

MCTPDriverMgr::~MCTPDriverMgr() {
    lg2::debug("Destructor MCTPDriverMgr");
    for (struct MCTPDriverContext drvCtx : _drvCtx) {
        const MCTPDriverIntf* drvIntf = drvCtx.intf;
        // TODO: Destruct something from struct MCTPDriverContext?
    }
    for (void* handle : _drvHandles) {
        dlclose(handle);
    }
}

bool MCTPDriverMgr::loadDriver(const std::string dyDriverPath) {
    lg2::debug("Try to open driver {PATH}", "PATH", dyDriverPath);
    void* handle = dlopen(dyDriverPath.c_str(), RTLD_LAZY);
    if (!handle) {
        lg2::error("Unable to dlopen driver {WHAT}", "WHAT", dlerror());
        return false;
    }

    dlerror();

    // Load the symbol (function)
    typedef MCTPDriverIntf* (*getDriver)();
    getDriver driverFactory = (getDriver)dlsym(handle, "getDriver");

    const char* error = dlerror();
    if (error) {
        lg2::error("Unable to getDriver {WHAT}", "WHAT", error);
        dlclose(handle);
        return false;
    }
    _drvHandles.push_back(handle);

    MCTPDriverIntf* drvIntf = driverFactory();

    if (drvIntf) {
        lg2::info("Added driver {DRIVER_NAME}", "DRIVER_NAME", drvIntf->getDriverUUID());
        struct MCTPDriverContext drvCtx = {drvIntf, handle};
        _drvCtx.push_back(drvCtx);
    }
    return true;
}

bool MCTPDriverMgr::bindDriver(MCTPEndPointBase* ep) {
    if (ep == nullptr) {
        lg2::error("ep is nullptr");
        return false;
    }

    for (struct MCTPDriverContext drvCtx : _drvCtx) {
        MCTPDriverIntf* drvIntf = drvCtx.intf;
        if (drvIntf && drvIntf->attachDevice(ep)){
            lg2::info("Net {NET} EId {EID} binded with the driver {NAME}.",
                "NET", ep->getNet(),
                "EID", ep->getEId(),
                "NAME", drvIntf->getDriverUUID());
        }
    }
    return true;
}
