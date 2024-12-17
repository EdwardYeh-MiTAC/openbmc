RMCPP_IFACE = "${@bb.utils.contains("MACHINE_FEATURES", "bonding", "bond0", "${DEFAULT_RMCPP_IFACE}", d)}"

SYSTEMD_SERVICE:${PN} += " \
        ${PN}@usb0.service \
        ${PN}@usb0.socket \
        "
SYSTEMD_SERVICE:${PN} += " \
        ${PN}@eth0.service \
        ${PN}@eth0.socket \
        "
SYSTEMD_SERVICE:${PN} += " \
        ${PN}@eth1.service \
        ${PN}@eth1.socket \
        "
