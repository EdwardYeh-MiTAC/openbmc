FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
inherit obmc-phosphor-systemd

RDEPENDS:${PN}:append = " bash"

SRC_URI:append = "\
    file://0001-Moved-the-declaration-of-static-unordered_map-from-f.patch \
    "
