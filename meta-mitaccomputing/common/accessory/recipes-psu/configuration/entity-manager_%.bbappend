FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_psu_generic.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${WORKDIR}/mitac_psu_generic.json ${D}${datadir}/${PN}/configurations/mitac_psu_generic.json
}
