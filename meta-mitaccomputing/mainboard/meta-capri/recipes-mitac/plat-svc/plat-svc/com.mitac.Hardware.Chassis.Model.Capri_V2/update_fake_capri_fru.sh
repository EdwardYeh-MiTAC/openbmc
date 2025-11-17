FRU_DEV=/sys/bus/i2c/devices/1-0050/eeprom
TargetPN="Capri V2"
SizeKB=1
if [ -e $FRU_DEV ]; then
	echo "Fount FRU device. Try to update FRU..."
        BoardPN=`fru-simple-read -i $FRU_DEV -b`
        FilePN=`fru-simple-read -i $1 -b`
        if [ "${BoardPN}" != "${FilePN}" ]; then
		echo "Product Name in FRU device: $BoardPN"
	        echo "Product Name in rom file: $FilePN"	
		read -p "Continue? (Y/N): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1
        	dd if=$1 of=$FRU_DEV bs=1k count=$SizeKB
        else
                echo "FRU Matched"
        fi
fi
