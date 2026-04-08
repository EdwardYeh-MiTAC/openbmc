FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_hsbp_dcphsbp2208.json \
    file://mitac_hsbp_M1333T76-BPE-2.json \
    file://mitac_hsbp_R1520G6HSBP204.json \
    file://mitac_hsbp_R1520G6HSBP210.json \
    file://mitac_hsbp_R1520G6HSBPE104.json \
    file://mitac_hsbp_R2520G6HSBP208.json \
    file://mitac_hsbp_R2520G6HSBPE108.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_hsbp_*.json ${D}${datadir}/${PN}/configurations/
}
