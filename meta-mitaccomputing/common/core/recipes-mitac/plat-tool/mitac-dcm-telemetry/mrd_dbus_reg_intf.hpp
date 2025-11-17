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
#include <sdbusplus/asio/property.hpp>

#include "mrd_reg_intf.hpp"

namespace dcm
{
namespace telemetry
{

class MRDDbusRegIntf : public MRDRegIntf
{
    public:
        virtual ~MRDDbusRegIntf() = default;
        explicit MRDDbusRegIntf();
        explicit MRDDbusRegIntf(std::shared_ptr<sdbusplus::asio::connection> pSdBusPlus);

        bool addMRD(const nlohmann::json& req) override;
        bool delMRD(const std::string& Id) override;
        bool refreshSensorList() override;
        bool isValidSensor(const std::string& sensorUri) override;

    private:
        bool getUserParameters(const nlohmann::json& req, AddReportArgs& args) const;
        std::shared_ptr<sdbusplus::asio::connection> pSdBusPlus;
        boost::asio::io_context io;
        const char* telemetrySrvName = "xyz.openbmc_project.Telemetry";
        const char* reportInterface = "xyz.openbmc_project.Telemetry.Report";
        const std::shared_ptr<sdbusplus::asio::connection> getDbusSrv();
        std::set<std::string> sensorList;
        std::string getDBusPath(const std::string& sensorUri);
};

}
}
