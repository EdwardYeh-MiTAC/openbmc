FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

RDEPENDS:${PN}:append = " bash"

SRC_URI:append = " \
    file://0001-Updated-legacy.json-to-support-MuxIdleMode-and-MuxCh.patch \
    file://0002-Support-MUXDEV-and-MUXCH-so-that-configuration-file-.patch \
    "
