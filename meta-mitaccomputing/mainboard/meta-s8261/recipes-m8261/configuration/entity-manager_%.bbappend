FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_s8261_baseboard.json \
    "

RDEPENDS:${PN}:append = " bash"


do_install:append() {
    install -m 0644 -D ${WORKDIR}/mitac_s8261_baseboard.json ${D}${datadir}/${PN}/configurations/
}
