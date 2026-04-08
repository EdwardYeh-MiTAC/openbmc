RMCPP_IFACE = "${@bb.utils.contains("MACHINE_FEATURES", "bonding", "bond0", "${DEFAULT_RMCPP_IFACE}", d)}"

SYSTEMD_SERVICE:${PN}:append = "\
    ${PN}@eth0.service \
    ${PN}@eth0.socket \
    "

SYSTEMD_SERVICE:${PN}:append = "\
    ${PN}@eth1.service \
    ${PN}@eth1.socket \
    "
