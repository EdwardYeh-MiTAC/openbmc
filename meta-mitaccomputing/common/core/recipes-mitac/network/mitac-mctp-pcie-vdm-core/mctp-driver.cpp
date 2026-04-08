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
#include <mctp-driver.hpp>

namespace com::mitac_computing::mctp
{

MCTPDriverHandle::MCTPDriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp)
	: _drvIntf(drvIntf), _mctpEp(mctpEp)
{

}

std::string MCTPDriverHandle::getDriverUUID() const {
	if (_drvIntf)
		return _drvIntf->getDriverUUID();
	else
		return "";
};

std::uint8_t MCTPDriverHandle::getNet() const {
	if (_mctpEp)
		return _mctpEp->getNet();
	return 0;
};

std::uint8_t MCTPDriverHandle::getEId() const {
	if (_mctpEp)
		return _mctpEp->getEId();
	return 0;
};

};
