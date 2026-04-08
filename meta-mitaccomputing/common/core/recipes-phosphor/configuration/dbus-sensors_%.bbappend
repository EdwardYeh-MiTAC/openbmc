FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:remove = " ipmbsensor"
PACKAGECONFIG:append = " nvmesensor"

SRC_URI:append = " \
    file://0001-Support-using-MuxChannel-modifier.patch \
    file://1001-Updated-PSUSensorMain-to-support-sensors-and-the-nam.patch \
    file://1002-Customized-IntelCPUSensor-to-follow-MiOBMC-naming-co.patch \
    file://1003-Customized-Fan-PresenceGpio-to-follow-MiOBMC-sensor-.patch \
    file://1004-Fixed-incorrect-health-status-from-OperationalStatus.patch \
    "
