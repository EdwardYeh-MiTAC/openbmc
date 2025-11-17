#!/bin/bash

HOST_COUNT=1
PLATFORM_COMPACT_NAME="com.mitac.Hardware.Chassis.Model.Template"

get_mac_addr_from_fru()
{
    echo "TODO: Get MAC Address [$1] from FRU."
}

exec_before_power_control () {
        echo "before_exec"
}

exec_after_power_control () {
        echo "after_exec"
}

exec_before_power_control () {
        echo "before_exec"
}

exec_after_power_control () {
        echo "after_exec"
}

action_power_transition_off () {
	echo "power transit to off"
}

action_power_status_off () {
	echo "powered off"
}
