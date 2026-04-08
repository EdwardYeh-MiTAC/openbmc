RDEPENDS:${PN}-extras:append = " \
    phosphor-image-signing \
    phosphor-virtual-sensor \
    phosphor-misc-usb-ctrl \
    phosphor-gpio-monitor-monitor \
    phosphor-gpio-monitor-presence \
    phosphor-hostlogger \
    phosphor-sel-logger \
    phosphor-logging \
    phosphor-post-code-manager \
    phosphor-host-postd \
    phosphor-software-manager \
    obmc-phosphor-buttons-signals \
    obmc-phosphor-buttons-handler \
    util-linux-logger \
    smbios-mdr \
    phosphor-ipmi-blobs \
    amd-apml \
    powercap-monitor \
    phosphor-snmp \
    net-snmp \
    amd-ras \
    cpu-info \
    "

RDEPENDS:${PN}-inventory:append = "\
    dbus-sensors \
    entity-manager \
    "

RDEPENDS:${PN}-extras:remove = " phosphor-hwmon"
VIRTUAL-RUNTIME_obmc-sensors-hwmon ?= " dbus-sensors"
