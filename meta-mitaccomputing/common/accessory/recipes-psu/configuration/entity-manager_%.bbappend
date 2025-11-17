FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_psu_generic.json \
    file://mitac_psu_murata-ps.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_psu_generic.json ${D}${datadir}/${PN}/configurations/
    install -m 0644 -D ${UNPACKDIR}/mitac_psu_murata-ps.json ${D}${datadir}/${PN}/configurations/
}
