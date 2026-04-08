FILESEXTRAPATHS:prepend:aspeed-g6 = "${THISDIR}/${PN}/AST2600:"
FILESEXTRAPATHS:prepend:aspeed-g7 = "${THISDIR}/${PN}/AST2700:"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit allarch systemd obmc-phosphor-systemd

RDEPENDS:${PN} += "bash"
RDEPENDS:${PN} += "libgpiod-tools"
RDEPENDS:${PN} += "mitac-common-functions"
IMAGE_INSTALL:append = " plat-svc"

SRC_URI:append= "\
    file://mitac-sys-init.service \
    file://mitac-sys-init-main \
    "

SRC_URI:append = "\
    file://mitac-power-transition-monitor.service \
    file://mitac-power-transition-monitor \
    "

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN}:append = "\
    mitac-sys-init.service \
    mitac-power-transition-monitor.service \
    "

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

do_install() {
    install -d ${D}${libexecdir}
    install -m 0755 ${UNPACKDIR}/mitac-sys-init-main ${D}${libexecdir}
    install -m 0755 ${UNPACKDIR}/mitac-power-transition-monitor ${D}${libexecdir}
}

