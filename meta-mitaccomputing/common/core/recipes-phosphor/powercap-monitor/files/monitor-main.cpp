#include "powercap-monitor.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>

int main()
{    auto bus = sdbusplus::bus::new_default();

    lg2::info("Starting PowerCap monitor...");

    try
    {
        phosphor::powercap::PowerCapMonitor monitor(bus);

        while (true)
        {
            bus.process_discard();
            bus.wait();
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Unhandled exception in PowerCapMonitor: {ERROR}", "ERROR", e.what());
        return 1;
    }

    return 0;
}
