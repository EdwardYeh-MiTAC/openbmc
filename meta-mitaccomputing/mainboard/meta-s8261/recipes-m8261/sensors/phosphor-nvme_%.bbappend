FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://nvme_config.json"

PLATFORM_COMPACT_NAME = "com.mitac.Hardware.Chassis.Model.S8261"

do_install:append() {
    install -d ${D}${sysconfdir}/nvme/${PLATFORM_COMPACT_NAME}
    install -m 0644 -D ${WORKDIR}/nvme_config.json ${D}/${sysconfdir}/nvme/${PLATFORM_COMPACT_NAME}
}
