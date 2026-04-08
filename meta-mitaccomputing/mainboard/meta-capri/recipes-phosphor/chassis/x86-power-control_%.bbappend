FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " bash"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.Capri_V2/ \
    "

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}${datadir}/${PN}/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/power-config-host0.json ${D}${datadir}/${PN}/${profile_name}/
    done
}
