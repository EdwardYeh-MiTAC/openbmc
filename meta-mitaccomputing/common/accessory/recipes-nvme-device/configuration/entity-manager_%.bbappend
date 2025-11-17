FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

SRC_URI:append= "\
    file://mitac_nvme_kioxia_generic.json \
    file://mitac_nvme_micron_generic.json \
    file://mitac_nvme_samsung_generic.json \
    file://mitac_nvme_solidigm_generic.json \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/mitac_nvme_kioxia_generic.json ${D}${datadir}/${PN}/configurations/
    install -m 0644 -D ${UNPACKDIR}/mitac_nvme_micron_generic.json ${D}${datadir}/${PN}/configurations/
    install -m 0644 -D ${UNPACKDIR}/mitac_nvme_samsung_generic.json ${D}${datadir}/${PN}/configurations/
    install -m 0644 -D ${UNPACKDIR}/mitac_nvme_solidigm_generic.json ${D}${datadir}/${PN}/configurations/
}
