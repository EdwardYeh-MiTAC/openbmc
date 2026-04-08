LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit allarch systemd obmc-phosphor-systemd

RDEPENDS:${PN} += "bash"
RDEPENDS:${PN} += "fbida"

SRC_URI += " \
    file://mitac-early-vga.service \
    file://MiTAC_Computing_800X600_Black.jpg \
    "

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} += " \
    mitac-early-vga.service \
    "

FILES:${PN}= "/usr/Logo"
S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

do_install() {
    install -d ${D}/usr/Logo
    install -m 0755 ${UNPACKDIR}/MiTAC_Computing_800X600_Black.jpg ${D}/usr/Logo
}

