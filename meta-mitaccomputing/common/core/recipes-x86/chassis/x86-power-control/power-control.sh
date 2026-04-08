#!/bin/bash

MAINBOARD_ENV=/tmp/mainboard_env.sh
DATA_PATH=/usr/share/x86-power-control
INIT_FLAG=/tmp/.x86-power-control.inited
source $MAINBOARD_ENV
echo "Platform Config: $PLATFORM_COMPACT_NAME"

if [ ! -e $INIT_FLAG ]; then
        CONFIG_PATH=$DATA_PATH/default
        if [ -d "$DATA_PATH/$PLATFORM_COMPACT_NAME" ]; then
                CONFIG_PATH="$DATA_PATH/$PLATFORM_COMPACT_NAME"
        fi
        echo "Create symbolic link"
        rm $DATA_PATH/power-config-host?.json
        for (( host_id=0; host_id<HOST_COUNT; host_id++ ))
        do
                ln -s $CONFIG_PATH/power-config-host$host_id.json \
                        $DATA_PATH/power-config-host$host_id.json
        done
        touch $INIT_FLAG
fi

exec_before_power_control
# workaround for core dump issue xyz.openbmc_project.Chassis.Buttons.service
GPIO_DEFS=/etc/default/obmc/gpio/gpio_defs.json
if [ ! -e $GPIO_DEFS ]; then
    touch $GPIO_DEFS
    echo '{' >> $GPIO_DEFS
    echo '"_comments": "This file should be overridden with one from the machine layer."' >> $GPIO_DEFS
    echo '}' >> $GPIO_DEFS
fi
/usr/bin/power-control $1
exec_after_power_control
