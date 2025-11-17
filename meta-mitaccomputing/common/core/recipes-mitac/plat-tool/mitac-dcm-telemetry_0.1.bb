LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
RDEPENDS:${PN}:append = " bash"

DEPENDS:append = " \
    boost \
    nlohmann-json \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    "

inherit pkgconfig systemd obmc-phosphor-systemd

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SRC_URI = " \
    file://mrd_dbus_reg_intf.cpp \
    file://mrd_dbus_reg_intf.hpp \
    file://mrd_reg_intf.hpp \
    file://mitac-dcm-telemetry.cpp \
    file://mitac-dcm-telemetry.hpp \
    file://mitac-dcm-telemetry.service \
    "

SYSTEMD_SERVICE:${PN}:append = " \
    mitac-dcm-telemetry.service \
    "

do_compile() {
    ${CXX} ${CFLAGS} ${LDFLAGS} mitac-dcm-telemetry.cpp mrd_dbus_reg_intf.cpp -std=c++23 -lboost_program_options -lsdbusplus -lsystemd -lphosphor_logging -o mitac-dcm-telemetry
}

do_install() {
    install -d ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/mitac-dcm-telemetry ${D}${base_bindir}
}
