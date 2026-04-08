SUMMARY = "Monitor D-Bus PowerCap property and apply APML settings to AMD CPU"
DESCRIPTION = "A system service that monitors D-Bus PowerCap property changes \
and uses apml_tool to apply dynamic power-related settings to AMD CPUs \
via the APML interface."

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "file://powercap-monitor.cpp \
           file://powercap-monitor.hpp \
           file://monitor-main.cpp \
           file://meson.build \
           file://powercap-monitor.service"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += "sdbusplus amd-apml phosphor-logging phosphor-dbus-interfaces systemd"

inherit meson systemd pkgconfig

SYSTEMD_SERVICE:${PN} = "powercap-monitor.service"
SYSTEMD_AUTO_ENABLE:${PN} = "disable"

do_install:append() {
    install -Dm644 ${UNPACKDIR}/powercap-monitor.service ${D}${systemd_system_unitdir}/powercap-monitor.service
}
