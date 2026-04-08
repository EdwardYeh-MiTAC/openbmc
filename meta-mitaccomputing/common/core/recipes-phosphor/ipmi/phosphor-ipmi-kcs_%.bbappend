# Create additional kcsbridge service on /dev/ipmi-kcs1
KCS_DEVICE_ALT = "ipmi-kcs3"
SYSTEMD_SERVICE:${PN}:append = " ${PN}@${KCS_DEVICE_ALT}.service"
