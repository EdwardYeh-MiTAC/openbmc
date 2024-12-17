#!/bin/bash
LIBPATH=/usr/libexec
source $LIBPATH/mitac-common-functions
get_mac_addr_from_fru 0 MACADDR0

BONDDEV_CFG_PATH=/etc/systemd/network/10-bmc-bond0.netdev

#To use eth0 mac address. Get mac address directly from FRU to avoid surprise from sysfs.
#ETH0_MAC=`cat /sys/class/net/eth0/address`
ETH0_MAC=$MACADDR0

VALID_MAC_CONFIG=$(echo MACAddress=$ETH0_MAC)
if [ -e $BONDDEV_CFG_PATH ]; then
	if grep -q $VALID_MAC_CONFIG "$BONDDEV_CFG_PATH"; then
		echo "Has valid mac address."
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
