FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
PACKAGECONFIG:append = " dynamic-sensors entity-manager-decorators arm-sbmr "

SRC_URI:append = " \
    file://0001-Use-MiOBMC-s-naming-convention-to-preset-EntityID-an.patch \
    file://0002-Skip-malformed-IPMI-Decorators-to-improve-the-robust.patch \
"
