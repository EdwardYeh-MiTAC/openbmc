FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd
EXTRA_OECONF = "--enable-configure-dbus=yes"

SYSTEMD_SERVICE:${PN} = "phosphor-pid-control.service"

SRC_URI += " \
    file://mitac-fan-table-init-main \
    file://0001-Present-sensor-name-when-coredump-is-caused-by-inval.patch \
    file://0002-Provides-indication-for-debug-of-redundancy-failure.patch \
    "

RDEPENDS:${PN} += "bash"

FILES:${PN}:append = " ${libexecdir}/mitac-fan-table-init-main"

do_install:append() {
    install -d ${D}${libexecdir}
    install -m 0755 ${WORKDIR}/mitac-fan-table-init-main ${D}${libexecdir}
}
