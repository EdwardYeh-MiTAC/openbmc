LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
RDEPENDS:${PN}:append = " bash mitac-mctp-pcie-vdm-core"

DEPENDS:append = " \
    boost \
    nlohmann-json \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    i2c-tools \
    mitac-mctp-pcie-vdm-core \
    "

inherit pkgconfig

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SRC_URI = " \
	file://mctp-spdm-generic-driver.hpp \
	file://mctp-spdm-generic-driver.cpp \
	file://mctp-nvme-mi-generic-driver.hpp \
	file://mctp-nvme-mi-generic-driver.cpp \
    "

do_compile() {
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        mctp-spdm-generic-driver.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o mctp-spdm-generic-driver.o

    ${CXX} ${CFLAGS} ${LDFLAGS} \
        -std=c++23 \
        -shared -o libmctp-spdm-generic-driver.so \
                    mctp-spdm-generic-driver.o

    ${CXX} ${CFLAGS} ${LDFLAGS} \
        mctp-nvme-mi-generic-driver.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o mctp-nvme-mi-generic-driver.o

    ${CXX} ${CFLAGS} ${LDFLAGS} \
        -std=c++23 \
        -shared -o libmctp-nvme-mi-generic-driver.so \
                    mctp-nvme-mi-generic-driver.o

#    ${CXX} ${CFLAGS} ${LDFLAGS} \
#        brcm-vdm-storelib7-driver.cpp \
#        -std=c++23 \
#        -c \
#        -fPIC \
#        -o brcm-vdm-storelib7-driver.o

#    ${CXX} ${CFLAGS} ${LDFLAGS} \
#        -std=c++23 \
#        -shared -o libbrcm-vdm-storelib7-driver.so \
#                    brcm-vdm-storelib7-driver.o
}

do_install() {
    install -d ${D}/${libdir}/${PN}
    install -m 0755 ${UNPACKDIR}/libmctp-spdm-generic-driver.so ${D}/${libdir}/${PN}
    install -m 0755 ${UNPACKDIR}/libmctp-nvme-mi-generic-driver.so ${D}/${libdir}/${PN}
#    install -m 0755 ${UNPACKDIR}/libbrcm-vdm-storelib7-driver.so ${D}/${libdir}/${PN}
}
