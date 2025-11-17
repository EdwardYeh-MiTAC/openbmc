MITAC_POSTCODE_DEV="/dev/mitac-postcode-dev"
POSTCODE_DEV1="/dev/aspeed-lpc-snoop0"
POSTCODE_DEV2="/dev/aspeed-lpc-pcc0"

if [ ! -e $MITAC_POSTCODE_DEV ]; then
        echo "$MITAC_POSTCODE_DEV not found."
        if [ -e $POSTCODE_DEV1 ]; then
                echo "Use $POSTCODE_DEV1 as post code device"
                ln -s $POSTCODE_DEV1 $MITAC_POSTCODE_DEV

		/usr/bin/snoopd -b 2 -d $MITAC_POSTCODE_DEV --rate-limit=1000
        elif [ -e $POSTCODE_DEV2 ]; then
                echo "Use $POSTCODE_DEV2 as post code device"
                ln -s $POSTCODE_DEV2 $MITAC_POSTCODE_DEV

		/usr/bin/snoopd -b 4 -d $MITAC_POSTCODE_DEV --rate-limit=1000
        else
                echo "All postcode device candidate were not found."
        fi
else
        echo "$MITAC_POSTCODE_DEV is available"

fi

