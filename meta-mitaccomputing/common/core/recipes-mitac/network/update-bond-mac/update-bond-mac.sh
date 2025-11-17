#!/bin/bash
LIBPATH=/usr/libexec
source /tmp/mainboard_env.sh
get_mac_addr_from_fru 0 MACADDR0
MACADDR0="${MACADDR0,,}"

FRU_IS_INVALID=TRUE

BONDDEV_CFG_PATH=/etc/systemd/network/10-bmc-bond0.netdev

#To use eth0 mac address. Get mac address directly from FRU to avoid surprise from sysfs.
ETH0_MAC=`cat /sys/class/net/eth0/address`

if [ "$MACADDR0" = 00:00:00:00:00:00 ] || [ "$MACADDR0" = ff:ff:ff:ff:ff:ff ]; then
        echo "ERROR: FRU has incorrect MAC Address $MACADDR0. Will use Random MAC"
        VALID_MAC_CONFIG=$(echo MACAddress=$ETH0_MAC)
else
        echo "FRU has correct MAC Address."
        FRU_IS_INVALID=FALSE
        VALID_MAC_CONFIG=$(echo MACAddress=$MACADDR0)
fi

if [ -e $BONDDEV_CFG_PATH ]; then
        if grep -q $VALID_MAC_CONFIG "$BONDDEV_CFG_PATH"; then
                echo "Has matched MAC address."
        else
                if grep -q MACAddress "$BONDDEV_CFG_PATH"; then
                        echo "Update to valid mac address."
                        sed -i '/MACAddress/c\'"$VALID_MAC_CONFIG"'' $BONDDEV_CFG_PATH
                else
                        echo "Insert mac address configuration."
                        sed -i '/Kind=bond/c\Kind=bond\n'"$VALID_MAC_CONFIG"'' $BONDDEV_CFG_PATH
                fi

                #Restart bond0 to adapt new mac address
                ip link delete dev bond0
                systemctl restart systemd-networkd
        fi
else
        echo "Can't find $BONDDEV_CFG_PATH. Bond interface may not activated."
fi

if [ "$FRU_IS_INVALID" = "TRUE" ]; then
        echo "Unable to get correct MAC address. Need to investigate FRU configuration."
        exit -1;
fi
