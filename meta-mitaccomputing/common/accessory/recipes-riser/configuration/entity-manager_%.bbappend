FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_riser_dcp2u1sriser1std.json \
    file://mitac_riser_dcp2u1sriser3.json \
    file://mitac_riser_dcp2u1sriser4.json \
    file://mitac_riser_dcp2u1sriser5cem.json \
    file://mitac_riser_R1520G61SRISER2.json \
    file://mitac_riser_R1520G61SRISER5CEM.json \
    file://mitac_riser_R2520G61SRISER1GPU.json \
    file://mitac_riser_R2520G61SRISER1STD.json \
    file://mitac_riser_R2520G61SRISER3GPU.json \
    file://mitac_riser_R2520G61SRISER3.json \
    file://mitac_riser_R2520G61SRISER4.json \
    file://mitac_riser_R2520G61SRISER5CEM.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_riser_*.json ${D}${datadir}/${PN}/configurations/
}
