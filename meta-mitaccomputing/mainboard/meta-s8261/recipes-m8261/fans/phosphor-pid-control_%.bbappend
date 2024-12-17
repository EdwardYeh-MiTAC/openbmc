FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://thermal-mgmt-init-functions \
    file://phosphor-pid-control.json \
    "

PLATFORM_COMPACT_NAME = "com.mitac.Hardware.Chassis.Model.S8261"

RDEPENDS:${PN} += "bash"

FILES:${PN}:append = " ${libexecdir}/${PLATFORM_COMPACT_NAME}/thermal-mgmt-init-functions"
FILES:${PN}:append = " ${libexecdir}/${PLATFORM_COMPACT_NAME}/phosphor-pid-control.json"

do_install:append() {
    install -d ${D}${libexecdir}/${PLATFORM_COMPACT_NAME}/
    install -m 0755 ${WORKDIR}/thermal-mgmt-init-functions ${D}${libexecdir}/${PLATFORM_COMPACT_NAME}/
    install -m 0755 ${WORKDIR}/phosphor-pid-control.json ${D}${libexecdir}/${PLATFORM_COMPACT_NAME}/
}
