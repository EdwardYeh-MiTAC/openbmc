FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Ignore-ethernet-interface-when-it-is-running-on-NC-S.patch \
"
