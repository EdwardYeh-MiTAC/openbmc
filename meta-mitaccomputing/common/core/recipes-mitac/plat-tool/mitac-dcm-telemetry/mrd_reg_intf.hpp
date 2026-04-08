#pragma once
#include <nlohmann/json.hpp>

namespace dcm
{
namespace telemetry
{
struct AddReportArgs
{
    struct MetricArgs
    {
        std::vector<std::string> uris;
        std::string metricId;
        std::string collectionFunction;
        std::string collectionTimeScope;
        uint64_t collectionDuration = 0;
    };

    std::string id;
    std::string name;
    std::string reportingType;
    std::string reportUpdates;
    uint64_t appendLimit = std::numeric_limits<uint64_t>::max();
    std::vector<std::string> reportActions;
    uint64_t interval = std::numeric_limits<uint64_t>::max();
    std::vector<MetricArgs> metrics;
    bool metricReportDefinitionEnabled = true;
};

class MRDRegIntf
{
    public:
        virtual ~MRDRegIntf() = default;
        virtual bool addMRD(const nlohmann::json& req) = 0;
        virtual bool delMRD(const std::string& Id) = 0;
        virtual bool refreshSensorList() = 0;
        virtual bool isValidSensor(const std::string& sensorUri) = 0;
};

}
}
