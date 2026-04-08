SUMMARY = "PMCI Launcher"
DESCRIPTION = "Support to launch pmci services on-demand"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "git://git@github.com/Intel-BMC/pmci-launcher.git;protocol=ssh;branch=main"
SRCREV = "5b29e915cf8c34f84a6d4c03bd7fccf1a0642398"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

inherit cmake systemd

DEPENDS:append = " \
    systemd \
    sdbusplus \
    phosphor-logging \
    boost \
    "

SRC_URI:append = "\
    file://xyz.openbmc_project.pmci-launcher.service \
    file://0001-Suppress-the-build-error-null-dereference-from-compi.patch \
    "

FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.pmci-launcher.service"
FILES:${PN} += "${bindir}/pmci_launcher"
SYSTEMD_SERVICE:${PN} = "xyz.openbmc_project.pmci-launcher.service"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/pmci_launcher ${D}${bindir}/

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${UNPACKDIR}/xyz.openbmc_project.pmci-launcher.service ${D}${systemd_system_unitdir}
}

