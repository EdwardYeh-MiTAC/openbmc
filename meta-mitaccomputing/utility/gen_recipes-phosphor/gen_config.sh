rm -rf recipes-phosphor
cp -rf recipes-phosphor.template recipes-phosphor 

cd recipes-phosphor

platforms=( \
	com.mitac.Hardware.Chassis.Model.E7142 \
	com.mitac.Hardware.Chassis.Model.S8051 \
	)

modules=( \
	chassis/x86-power-control \
	configuration/entity-manager \
	fans/phosphor-fan \
	inventory/phosphor-inventory-manager \
	leds/phosphor-led-manager \
	sensors/phosphor-nvme \
	)

for module_dir in "${modules[@]}"
do
	for PLATFORM_NAME in "${platforms[@]}"
	do
		project_dir=$module_dir/$PLATFORM_NAME
		if [ ! -d "$project_dir" ]; then
			mkdir -p $project_dir
		fi
	done
done

bbappends=( \
	chassis/x86-power-control_%.bbappend \
	configuration/entity-manager_%.bbappend \
	fans/phosphor-fan_%.bbappend \
	inventory/phosphor-inventory-manager_%.bbappend \
	leds/phosphor-led-manager_%.bbappend\
	sensors/phosphor-nvme_%.bbappend \
	)

for bbappend_file in "${bbappends[@]}"
do
	platform_list="SRC_URI:append = \" "'\\'
	for PLATFORM_NAME in "${platforms[@]}"
	do
		platform_list="${platform_list}\n\t""file://${PLATFORM_NAME}/ "'\\'
	done
	#echo -e "SRC_URI:append = \" ${platform_list} \"" >> $bbappend_file
	platform_list="${platform_list}\n\t\""	
	echo -e "${platform_list}" >> $bbappend_file
done
