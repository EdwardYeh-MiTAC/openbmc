FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_dcscm_e7142dcscmv2.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_dcscm_*.json ${D}${datadir}/${PN}/configurations/
}
