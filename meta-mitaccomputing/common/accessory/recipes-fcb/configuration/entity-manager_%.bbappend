FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_fcb_dcpfanbp2u.json \
    file://mitac_fcb_r1520g6fanbp-2j.json \
    file://mitac_fcb_r1520g6fanbp-62.json \
    file://mitac_fcb_r2520g6fanbp-2l.json \
    file://mitac_fcb_r2520g6fanbp-5t.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_fcb_*.json ${D}${datadir}/${PN}/configurations/
}
