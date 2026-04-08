#pragma once

#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace phosphor
{
namespace powercap
{

class PowerCapMonitor
{
  public:
    PowerCapMonitor(sdbusplus::bus_t& bus);

  private:
    sdbusplus::bus_t& bus;
    sdbusplus::bus::match_t matchPowerCap;
    sdbusplus::bus::match_t matchOsState;

    bool cpu0Ok;
    bool cpu1Ok;

    void onPowerCapChanged(sdbusplus::message_t& msg);
    void onOsStateChanged(sdbusplus::message_t& msg);

    void setPowerCapEnable(bool enable);
};

} // namespace powercap
} // namespace phosphor
