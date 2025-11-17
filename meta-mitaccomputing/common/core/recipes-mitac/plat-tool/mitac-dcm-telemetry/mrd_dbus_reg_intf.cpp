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

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <sdbusplus/bus/match.hpp>

#include <optional>
#include <iostream>
#include <charconv>

#include "mrd_dbus_reg_intf.hpp"

namespace dcm
{
namespace telemetry
{

using ReadingParameters = std::vector<std::tuple<
    std::vector<std::tuple<sdbusplus::message::object_path, std::string>>,
    std::string, std::string, uint64_t>>;

using GetSubTreeType = std::vector<std::pair<
    std::string,
    std::vector<std::pair<std::string, std::vector<std::string>>>>>;

inline std::string toDbusCollectionFunction(std::string_view redfishValue)
{
    if (redfishValue == "Maximum")
    {
        return "xyz.openbmc_project.Telemetry.Report.OperationType.Maximum";
    }
    if (redfishValue == "Minimum")
    {
        return "xyz.openbmc_project.Telemetry.Report.OperationType.Minimum";
    }
    if (redfishValue == "Average")
    {
        return "xyz.openbmc_project.Telemetry.Report.OperationType.Average";
    }
    if (redfishValue == "Summation")
    {
        return "xyz.openbmc_project.Telemetry.Report.OperationType.Summation";
    }
    lg2::warning("Unsupported value: {VALUE}", "VALUE", redfishValue);
    return "";
}

inline std::string toDbusReportAction(std::string_view redfishValue)
{
    if (redfishValue == "RedfishEvent")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportActions.EmitsReadingsUpdate";
    }
    if (redfishValue == "LogToMetricReportsCollection")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportActions.LogToMetricReportsCollection";
    }
    lg2::warning("Unsupported value: {VALUE}", "VALUE", redfishValue);
    return "";
}

inline std::string toDbusReportingType(std::string_view redfishValue)
{
    if (redfishValue == "OnChange")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportingType.OnChange";
    }
    if (redfishValue == "OnRequest")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportingType.OnRequest";
    }
    if (redfishValue == "Periodic")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportingType.Periodic";
    }
    lg2::warning("Unsupported value: {VALUE}", "VALUE", redfishValue);
    return "";
}

inline std::string toDbusCollectionTimeScope(std::string_view redfishValue)
{
    if (redfishValue == "Point")
    {
        return "xyz.openbmc_project.Telemetry.Report.CollectionTimescope.Point";
    }
    if (redfishValue == "Interval")
    {
        return "xyz.openbmc_project.Telemetry.Report.CollectionTimescope.Interval";
    }
    if (redfishValue == "StartupInterval")
    {
        return "xyz.openbmc_project.Telemetry.Report.CollectionTimescope.StartupInterval";
    }
    lg2::warning("Unsupported value: {VALUE}", "VALUE", redfishValue);
    return "";
}

inline std::string toDbusReportUpdates(std::string_view redfishValue)
{
    if (redfishValue == "Overwrite")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportUpdates.Overwrite";
    }
    if (redfishValue == "AppendWrapsWhenFull")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportUpdates.AppendWrapsWhenFull";
    }
    if (redfishValue == "AppendStopsWhenFull")
    {
        return "xyz.openbmc_project.Telemetry.Report.ReportUpdates.AppendStopsWhenFull";
    }
    lg2::warning("Unsupported value: {VALUE}", "VALUE", redfishValue);
    return "";
}

inline bool toDbusReportActions(const std::vector<std::string>& actions,
                                std::vector<std::string>& outReportActions)
{
    size_t index = 0;
    for (const std::string& action : actions)
    {
        std::string dbusReportAction = toDbusReportAction(action);
        if (dbusReportAction.empty())
        {
            lg2::error("dbusReportAction is empty");
            return false;
        }
        outReportActions.emplace_back(std::move(dbusReportAction));
        index++;
    }
    return true;
}

inline std::optional<std::chrono::milliseconds>
    fromDurationString(std::string_view v)
{
    std::chrono::milliseconds out = std::chrono::milliseconds::zero();
    enum class ProcessingStage
    {
        // P1DT1H1M1.100S
        P,
        Days,
        Hours,
        Minutes,
        Seconds,
        Milliseconds,
        Done,
    };
    ProcessingStage stage = ProcessingStage::P;

    while (!v.empty())
    {
        if (stage == ProcessingStage::P)
        {
            if (v.front() != 'P')
            {
                return std::nullopt;
            }
            v.remove_prefix(1);
            stage = ProcessingStage::Days;
            continue;
        }
        if (stage == ProcessingStage::Days)
        {
            if (v.front() == 'T')
            {
                v.remove_prefix(1);
                stage = ProcessingStage::Hours;
                continue;
            }
        }
        uint64_t ticks = 0;
        auto [ptr, ec] = std::from_chars(v.begin(), v.end(), ticks);
        if (ec != std::errc())
        {
            lg2::error("Process ticks failed");
            return std::nullopt;
        }
        size_t charactersRead = static_cast<size_t>(ptr - v.data());
        if (ptr >= v.end())
        {
            lg2::error("Process duration failed");
            return std::nullopt;
        }
        if (*ptr == 'D')
        {
            if (stage > ProcessingStage::Days)
            {
                return std::nullopt;
            }
            out += std::chrono::days(ticks);
        }
        else if (*ptr == 'H')
        {
            if (stage > ProcessingStage::Hours)
            {
                return std::nullopt;
            }
            out += std::chrono::hours(ticks);
        }
        else if (*ptr == 'M')
        {
            if (stage > ProcessingStage::Minutes)
            {
                return std::nullopt;
            }
            out += std::chrono::minutes(ticks);
        }
        else if (*ptr == '.')
        {
            if (stage > ProcessingStage::Seconds)
            {
                return std::nullopt;
            }
            out += std::chrono::seconds(ticks);
            stage = ProcessingStage::Milliseconds;
        }
        else if (*ptr == 'S')
        {
            // We could be seeing seconds for the first time, (as is the case in
            // 1S) or for the second time (in the case of 1.1S).
            if (stage <= ProcessingStage::Seconds)
            {
                out += std::chrono::seconds(ticks);
                stage = ProcessingStage::Milliseconds;
            }
            else if (stage > ProcessingStage::Milliseconds)
            {
                lg2::error("Process duration failed");
                return std::nullopt;
            }
            else
            {
                // Seconds could be any form of (1S, 1.1S, 1.11S, 1.111S);
                // Handle them all milliseconds are after the decimal point,
                // so they need right padded.
                if (charactersRead == 1)
                {
                    ticks *= 100;
                }
                else if (charactersRead == 2)
                {
                    ticks *= 10;
                }
                out += std::chrono::milliseconds(ticks);
                stage = ProcessingStage::Milliseconds;
            }
        }
        else
        {
            lg2::error("Process duration failed");
            return std::nullopt;
        }

        v.remove_prefix(charactersRead + 1U);
    }
    return out;
}

inline bool getUserMetric(const nlohmann::json::object_t& metric,
                          AddReportArgs::MetricArgs& metricArgs)
{
    std::optional<std::vector<std::string>> uris;
    std::optional<std::string> metricIdStr;
    std::optional<std::string> collectionDurationStr;
    std::optional<std::string> collectionFunction;
    std::optional<std::string> collectionTimeScopeStr;

    if (metric.contains("MetricProperties"))
        uris = metric.at("MetricProperties");
    if (metric.contains("CollectionFunction"))
        collectionFunction = metric.at("CollectionFunction");
    if (metric.contains("CollectionTimeScope"))
        collectionTimeScopeStr = metric.at("CollectionTimeScope");
    if (metric.contains("CollectionDuration"))
        collectionDurationStr = metric.at("CollectionDuration");
    if (metric.contains("MetricId"))
        metricIdStr = metric.at("MetricId");

    if (uris)
    {
        metricArgs.uris = std::move(*uris);
    }

    if (metricIdStr)
    {
        metricArgs.metricId = *metricIdStr;
    }

    if (collectionFunction)
    {
        std::string dbusCollectionFunction =
            telemetry::toDbusCollectionFunction(*collectionFunction);
        if (dbusCollectionFunction.empty())
        {
            lg2::error("Process CollectionFunction failed");
            return false;
        }
        metricArgs.collectionFunction = std::move(dbusCollectionFunction);
    }

    if (collectionTimeScopeStr)
    {
        std::string dbusCollectionTimeScope =
            toDbusCollectionTimeScope(*collectionTimeScopeStr);
        if (dbusCollectionTimeScope.empty())
        {
            lg2::error("Process CollectionTimeScope failed");
            return false;
        }
        metricArgs.collectionTimeScope = std::move(dbusCollectionTimeScope);
    }

    if (collectionDurationStr)
    {
        std::optional<std::chrono::milliseconds> duration =
            fromDurationString(*collectionDurationStr);

        if (!duration || duration->count() < 0)
        {
            lg2::error("Process CollectionDuration failed");
            return false;
        }

        metricArgs.collectionDuration =
            static_cast<uint64_t>(duration->count());
    }

    return true;
}

inline bool getUserMetrics(std::span<nlohmann::json::object_t> metrics,
                           std::vector<AddReportArgs::MetricArgs>& result)
{
    result.reserve(metrics.size());

    for (nlohmann::json::object_t& m : metrics)
    {
        AddReportArgs::MetricArgs metricArgs;

        if (!getUserMetric(m, metricArgs))
        {
            lg2::error("Process UserMetric failed");
            return false;
        }

        result.emplace_back(std::move(metricArgs));
    }

    return true;
}

bool MRDDbusRegIntf::getUserParameters(const nlohmann::json& req, AddReportArgs& args) const
{
    std::optional<std::string> id;
    std::optional<std::string> reportingTypeStr;
    std::optional<std::string> reportUpdatesStr;
    std::optional<uint64_t> appendLimit;
    std::optional<bool> metricReportDefinitionEnabled;
    std::optional<std::vector<nlohmann::json::object_t>> metrics;
    std::optional<std::vector<std::string>> reportActionsStr;
    std::optional<std::string> scheduleDurationStr;

    try
    {
        id = req.at("Id");
        lg2::info("Handle MRD: {ID}", "ID", *id);
        metrics = req.at("Metrics");
        reportingTypeStr = req.at("MetricReportDefinitionType");
        reportActionsStr = req.at("ReportActions");
        if (req.contains("AppendLimit"))
            appendLimit = req.at("AppendLimit");
        if (req.contains("ReportUpdates"))
            reportUpdatesStr = req.at("ReportUpdates");
        if (req.contains("Schedule"))
        {
            nlohmann::json schedule = req.at("Schedule");
            scheduleDurationStr = schedule.at("RecurrenceInterval");
        }
        if (req.contains("MetricReportDefinitionEnabled"))
            metricReportDefinitionEnabled = req.at("MetricReportDefinitionEnabled");
    }
    catch (const std::exception& ex)
    {
        lg2::error("Exception: {WHAT}", "WHAT", ex.what());
    }

    if (id)
    {
        constexpr const char* allowedCharactersInId =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        if (id->empty() ||
            id->find_first_not_of(allowedCharactersInId) != std::string::npos)
        {
            lg2::error("Property Id is mandatory");
            return false;
        }
        args.id = *id;
    }

    if (reportingTypeStr)
    {
        std::string dbusReportingType = toDbusReportingType(*reportingTypeStr);
        if (dbusReportingType.empty())
        {
            lg2::error("Property MetricReportDefinitionType is mandatory");
            return false;
        }
        args.reportingType = dbusReportingType;
    }

    if (reportUpdatesStr)
    {
        std::string dbusReportUpdates = toDbusReportUpdates(*reportUpdatesStr);
        if (dbusReportUpdates.empty())
        {
            lg2::error("Property ReportUpdates is mandatory");
            return false;
        }
        args.reportUpdates = dbusReportUpdates;
    }

    if (appendLimit)
    {
        args.appendLimit = *appendLimit;
    }

    if (metricReportDefinitionEnabled)
    {
        args.metricReportDefinitionEnabled = *metricReportDefinitionEnabled;
    }

    if (reportActionsStr)
    {
        if (!toDbusReportActions(*reportActionsStr, args.reportActions))
        {
            lg2::error("Property ReportActionss is mandatory");
            return false;
        }
    }

    if (reportingTypeStr == "Periodic")
    {
        if (!scheduleDurationStr)
        {
            lg2::error("Property Schedule.Duration is mandatory for Periodic MRD type.");
            return false;
        }

        std::optional<std::chrono::milliseconds> durationNum =
            fromDurationString(*scheduleDurationStr);
        if (!durationNum || durationNum->count() < 0)
        {
            lg2::error("Property Schedule.Duration is invalid.");
            return false;
        }
        args.interval = static_cast<uint64_t>(durationNum->count());
    }

    if (metrics)
    {
        if (!getUserMetrics(*metrics, args.metrics))
        {
            lg2::error("Property Metrics is mandatory");
            return false;
        }
    }

    return true;
}


bool MRDDbusRegIntf::addMRD(const nlohmann::json& req)
{
    lg2::info("Enter addMRD");
    AddReportArgs args;
    ReadingParameters readingParams;
    if (getUserParameters(req, args))
    {
        readingParams.reserve(args.metrics.size());

        for (const auto& metric : args.metrics)
        {
            std::vector<
                std::tuple<sdbusplus::message::object_path, std::string>>
                sensorParams;
            sensorParams.reserve(metric.uris.size());

            for (size_t i = 0; i < metric.uris.size(); i++)
            {
                const std::string sensorUri = metric.uris[i];
                if (isValidSensor(sensorUri))
                {
                    const std::string dbusPath = getDBusPath(sensorUri);
                    sensorParams.emplace_back(dbusPath, sensorUri);
                }
                else
                {
                    lg2::warning("Drop sensor {URI} as absent.", "URI", metric.uris[i]);
                }
            }

            // ReadingParams is currently not support MetricID
            // https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/xyz/openbmc_project/Telemetry/ReportManager.interface.yaml#L48C19-L48C36
            readingParams.emplace_back(
                std::move(sensorParams), metric.collectionFunction,
                metric.collectionTimeScope, metric.collectionDuration);
        }
 

        auto addReportReq = getDbusSrv()->new_method_call(
            telemetrySrvName, "/xyz/openbmc_project/Telemetry/Reports",
            "xyz.openbmc_project.Telemetry.ReportManager", "AddReport");

       	addReportReq.append(
            "TelemetryService/" + args.id, args.name, args.reportingType,
            args.reportUpdates, args.appendLimit, args.reportActions,
            args.interval, readingParams, args.metricReportDefinitionEnabled);

        try
        {
            auto respMsg = getDbusSrv()->call(addReportReq);
            return true;
        }
        catch (const std::exception& ex)
        {
            lg2::error("Exception: {WHAT}", "WHAT", ex.what());
            return false;
        }
    }
    else
    {
        lg2::warning("getUserParameters retrun failed");
        return false;
    }
}

bool MRDDbusRegIntf::delMRD(const std::string& Id)
{
    lg2::info("Enter delMRD: MRD = {ID}", "ID", Id);
    const std::string& mdrPath = "/xyz/openbmc_project/Telemetry/Reports/TelemetryService/" + Id;
    auto delReportReq = getDbusSrv()->new_method_call(
        telemetrySrvName, mdrPath.c_str(),
        "xyz.openbmc_project.Object.Delete", "Delete");
    try
    {
        auto respMsg = getDbusSrv()->call(delReportReq);
        return true;
    }
    catch (const std::exception& ex)
    {
        lg2::error("Exception: {WHAT}", "WHAT", ex.what());
        return false;
    }
}

const std::shared_ptr<sdbusplus::asio::connection> MRDDbusRegIntf::getDbusSrv()
{
    if (!pSdBusPlus)
    {
        pSdBusPlus = std::make_shared<sdbusplus::asio::connection>(io);
    }

    return pSdBusPlus;
}

bool MRDDbusRegIntf::refreshSensorList()
{
    try
    {
        sensorList.clear();
        lg2::info("Collect sensor list.");
        auto getSubTreeReq = pSdBusPlus->new_method_call(
            "xyz.openbmc_project.ObjectMapper", "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTree");
        int32_t depth = 2;
        constexpr std::array<std::string_view, 1> interfaces{
            "xyz.openbmc_project.Sensor.Value"};
        getSubTreeReq.append("/xyz/openbmc_project/sensors", depth, interfaces);
        auto respMsg = pSdBusPlus->call(getSubTreeReq);
        GetSubTreeType data;
        respMsg.read(data);
        for (auto& item : data)
        {
            sensorList.insert(item.first);
        }
        return true;
    }
    catch (const std::exception& ex)
    {
        lg2::error("Exception: {WHAT}", "WHAT", ex.what());
        return false;
    }
}

bool MRDDbusRegIntf::isValidSensor(const std::string& sensorUri)
{
    const std::string dbusPath = getDBusPath(sensorUri);
    if (sensorList.contains(dbusPath))
        return true;

    return false;
}

std::string MRDDbusRegIntf::getDBusPath(const std::string& sensorUri)
{
    const std::string& sn = sensorUri.substr(sensorUri.rfind("/") + 1, sensorUri.length());
    const std::string& sensorType = sn.substr(0, sn.find('_'));
    const std::string& sensorName = sn.substr(sn.find('_') + 1, sn.length());
    /* May have mapping issue in this case:
    *   URI A: /redfish/v1/Chassis/{DEV_A}/Sensors/temperature_inlet_temp"
    *   URI B: /redfish/v1/Chassis/{DEV_B}/Sensors/temperature_inlet_temp"
    *  
    *   It will get the same dbusPath.
    */
    std::string dbusPath = "/xyz/openbmc_project/sensors/" + sensorType + "/" + sensorName;
    return dbusPath;
}

MRDDbusRegIntf::MRDDbusRegIntf():pSdBusPlus(nullptr)
{
}

MRDDbusRegIntf::MRDDbusRegIntf(std::shared_ptr<sdbusplus::asio::connection> conn):pSdBusPlus(conn)
{
}

} // namespace telemetry
} // namespace dcm
