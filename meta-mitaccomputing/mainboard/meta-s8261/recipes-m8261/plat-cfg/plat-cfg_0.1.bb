LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

RDEPENDS:${PN} += "bash"
RDEPENDS:${PN} += "libgpiod-tools"
RDEPENDS:${PN} += "mitac-common-functions"

SRC_URI += " \
    file://mainboard-init-functions \
    "

PLATFORM_COMPACT_NAME = "com.mitac.Hardware.Chassis.Model.S8261"

do_install() {
    install -d ${D}${libexecdir}/${PLATFORM_COMPACT_NAME}/
    install -m 0755 ${WORKDIR}/mainboard-init-functions ${D}${libexecdir}/${PLATFORM_COMPACT_NAME}/
}

