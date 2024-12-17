FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:remove = " intelcpusensor ipmbsensor"
PACKAGECONFIG:append = " nvmesensor"

SRC_URI:append= "\
    file://0001-Enable-the-support-for-tmp411-sensor.patch \
    "
