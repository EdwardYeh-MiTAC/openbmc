MCTPBUS=mctpi2c$1
DEFAULT_MTU=100

sync_mtu () {
	resp=`busctl --no-pager call \
    		xyz.openbmc_project.MCTP \
    		/xyz/openbmc_project/mctp au.com.CodeConstruct.MCTP \
    		SetupEndpoint say $MCTPBUS 1 0x1d`

	resp_arg=($resp)

        for i in "${resp_arg[@]}"
        do
                echo $i
        done

	EP_PATH=${resp_arg[3]}
	DEV_NET=${resp_arg[2]}
	DEV_EP=${resp_arg[1]}
	echo $EP_PATH

        echo "Sync MTU to $DEFAULT_MTU"
        busctl --no-pager call \
                xyz.openbmc_project.MCTP \
                /xyz/openbmc_project/mctp/$DEV_NET/$DEV_EP \
                au.com.CodeConstruct.MCTP.Endpoint \
                SetMTU u $DEFAULT_MTU

        mi-mctp $DEV_NET $DEV_EP set-config 1 mtu $DEFAULT_MTU
}

echo $1
if [ "$1" == "" ]; then
	echo "Usage: add_mctp_ep.sh BUS_NUM_MCTP_DEV"
	exit -1
fi

mctp address add 8 dev $MCTPBUS
mctp link set $MCTPBUS up
sync_mtu

echo "==================================="
echo " mi-mctp $DEV_NET $DEV_EP info     "
echo "==================================="
mi-mctp $DEV_NET $DEV_EP info

echo "==================================="
echo " mi-mctp $DEV_NET $DEV_EP controllers     "
echo "==================================="
mi-mctp $DEV_NET $DEV_EP controllers

echo "==================================="
echo " mi-mctp $DEV_NET $DEV_EP identify "
echo "==================================="
mi-mctp $DEV_NET $DEV_EP identify 1

echo "==================================="
echo " mi-mctp $DEV_NET $DEV_EP get-log-page 1 2 "
echo "==================================="
mi-mctp $DEV_NET $DEV_EP get-log-page 1 2

echo "==================================="
echo " nvme id-ctrl mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme id-ctrl mctp:$DEV_NET,$DEV_EP -H

echo "==================================="
echo " nvme list-ns mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme list-ns mctp:$DEV_NET,$DEV_EP

echo "==================================="
echo " nvme id-ns mctp:$DEV_NET,$DEV_EP --namespace-id=0x1"
echo "==================================="
nvme id-ns mctp:$DEV_NET,$DEV_EP --namespace-id=0x1 -H

echo "==================================="
echo " nvme show-regs -H mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme show-regs -H mctp:$DEV_NET,$DEV_EP

echo "==================================="
echo " nvme fw-log mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme fw-log mctp:$DEV_NET,$DEV_EP

echo "==================================="
echo " nvme smart-log mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme smart-log mctp:$DEV_NET,$DEV_EP

echo "==================================="
echo " nvme sanitize-log mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme sanitize-log mctp:$DEV_NET,$DEV_EP

echo "==================================="
echo " nvme supported-log-pages mctp:$DEV_NET,$DEV_EP"
echo "==================================="
nvme supported-log-pages mctp:$DEV_NET,$DEV_EP
