FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " json"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.S8050/ \
    file://com.mitac.Hardware.Chassis.Model.S8056/ \
    file://com.mitac.Hardware.Chassis.Model.S8261/ \
    file://com.mitac.Hardware.Chassis.Model.SC513G6/ \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}${datadir}/${PN}/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/*.json ${D}${datadir}/${PN}/${profile_name}/
    done
}
