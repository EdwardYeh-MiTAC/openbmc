FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:remove = " ipmbsensor"
PACKAGECONFIG:append = " nvmesensor"
