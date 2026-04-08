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

#include <nlohmann/json.hpp>

#include "mrd_reg_intf.hpp"

namespace dcm
{
namespace telemetry
{
enum class HostState
{
    /** Host CPU is powered off. */
    off,
    /** Host CPU is powered on, but BIOS has not completed POST. */
    postInProgress,
    /** BIOS has completed POST. */
    postComplete
};

class MRDLoader
{
    public:
        virtual ~MRDLoader() = default;
        explicit MRDLoader();
        explicit MRDLoader(const std::string config);
        
        bool parseMRDConfig(const std::string config);
        bool subscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf);
        bool unsubscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf);
        bool resubscribeMRD(dcm::telemetry::MRDRegIntf& mrdRegIntf);
    private:
        nlohmann::json mitacMRD;
        bool validateMRD(const nlohmann::json* mrd);
};

}
}
