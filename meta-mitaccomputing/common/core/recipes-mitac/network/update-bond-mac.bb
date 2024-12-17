LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit allarch systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"
RDEPENDS:${PN} += "bash"

SRC_URI +=" \
    file://update-bond-mac.sh \
    file://update-bond-mac.service \
    "

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} += " update-bond-mac.service"

do_install() {
    install -d ${D}${libexecdir}/${PN}
    install -m 0755 ${WORKDIR}/update-bond-mac.sh ${D}${libexecdir}/${PN}/update-bond-mac.sh

    install -d ${D}${systemd_unitdir}/system/
    install -m 0644 ${WORKDIR}/update-bond-mac.service ${D}${systemd_unitdir}/system
}
