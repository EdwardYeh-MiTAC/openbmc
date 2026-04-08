FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

RDEPENDS:${PN}:append = " bash"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.Capri_V2/ \
    "

SRC_URI:append = " \
    file://0001-Made-fru-device-utility-just-ignore-illegal-areas-wh.patch \
    "

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -m 0644 ${UNPACKDIR}/${profile_name}/*.json ${D}${datadir}/${PN}/configurations/
    done
}
