# phosphor led configuration
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " bash"
PACKAGECONFIG:append = " use-json use-lamp-test"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.S8050/ \
    file://com.mitac.Hardware.Chassis.Model.S8056/ \
    file://com.mitac.Hardware.Chassis.Model.S8261/ \
    file://com.mitac.Hardware.Chassis.Model.SC513G6/ \
    "

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}${datadir}/${PN}/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/led-group-config.json ${D}${datadir}/${PN}/${profile_name}/
    done
}
