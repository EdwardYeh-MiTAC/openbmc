FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " json"

SRC_URI:append = " \
                              file://monitor.json \
                              file://presence.json \
"

PLATFORM_COMPACT_NAME = "com.mitac.Hardware.Chassis.Model.S8261"

do_install:append () {
        install -d ${D}${datadir}/phosphor-fan-presence/monitor/${PLATFORM_COMPACT_NAME}
        install -d ${D}${datadir}/phosphor-fan-presence/presence/${PLATFORM_COMPACT_NAME}

        install -m 0644 ${WORKDIR}/monitor.json \
                ${D}${datadir}/phosphor-fan-presence/monitor/${PLATFORM_COMPACT_NAME}/config.json
        install -m 0644 ${WORKDIR}/presence.json \
                ${D}${datadir}/phosphor-fan-presence/presence/${PLATFORM_COMPACT_NAME}/config.json
}
