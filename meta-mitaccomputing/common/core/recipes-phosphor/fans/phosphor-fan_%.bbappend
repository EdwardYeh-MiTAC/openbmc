FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " json"

SRC_URI:append = " \
                              file://phosphor-fan-monitor@.service \
                              file://phosphor-fan-presence-tach@.service \
"
do_install:append () {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/phosphor-fan-monitor@.service ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/phosphor-fan-presence-tach@.service ${D}${systemd_system_unitdir}
}
