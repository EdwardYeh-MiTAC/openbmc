FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://bond-resolv.conf"

BONDING_CONF = "\
    file://10-bmc-bond0.netdev \
    file://10-bmc-bond0.network \
    file://20-bmc-eth0.network \
    file://30-bmc-eth1.network \
    "

SRC_URI:append = "${@bb.utils.contains('MACHINE_FEATURES', 'bonding',\
                        ' ${BONDING_CONF}', '', d)}"

FILES:${PN}:append = "\
    ${sysconfdir}/systemd/resolved.conf.d/bond-resolv.conf \
    "
BONDING_CONF_PATH = "\
    ${sysconfdir}/systemd/network/10-bmc-bond0.netdev \
    ${sysconfdir}/systemd/network/10-bmc-bond0.network \
    ${sysconfdir}/systemd/network/20-bmc-eth0.network \
    ${sysconfdir}/systemd/network/30-bmc-eth1.network \
    "
FILES:${PN}:append = "${@bb.utils.contains('MACHINE_FEATURES', 'bonding',\
                         ' ${BONDING_CONF_PATH}', '', d)}"

do_install:append() {
    install -d ${D}${sysconfdir}/systemd/resolved.conf.d/
    install -m 0644 ${WORKDIR}/bond-resolv.conf \
        ${D}${sysconfdir}/systemd/resolved.conf.d/
    install -d ${D}${sysconfdir}/systemd/network/
    if ${@bb.utils.contains('MACHINE_FEATURES', 'bonding', 'true', 'false', d)};
    then
        install -m 0644 ${WORKDIR}/10-bmc-bond0.netdev \
            ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/10-bmc-bond0.network \
            ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/20-bmc-eth0.network \
            ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/30-bmc-eth1.network \
            ${D}${sysconfdir}/systemd/network/
    fi
}
