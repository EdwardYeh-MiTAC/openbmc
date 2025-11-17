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
    i2c-tools \
    "

inherit pkgconfig

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SRC_URI = " \
    file://xdpe1x2xx_config.cpp \
    file://xdpe1x2xx_config.hpp \
    file://xdpe1x2xx_controller_intf.cpp \
    file://xdpe1x2xx_controller_intf.hpp \
    file://xdpe1x2xx_dump.cpp \
    file://test_xdpe1x2xx_dump.cpp \
    file://test_xdpe1x2xx_update.cpp \
    file://xdpe1x2xx_firmware_dump.cpp \
    file://xdpe1x2xx_firmware_dump.hpp \
    file://xdpe1x2xx_products.hpp \
    file://xdpe1x2xx_update.cpp \
    file://xdpe1x2xx_utility.cpp \
    file://xdpe1x2xx_utility.hpp \
    "

do_compile() {
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_config.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o xdpe1x2xx_config.o
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_firmware_dump.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o xdpe1x2xx_firmware_dump.o
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_utility.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o xdpe1x2xx_utility.o
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_controller_intf.cpp \
        -std=c++23 \
        -c \
        -fPIC \
        -o xdpe1x2xx_controller_intf.o

    ${CXX} ${CFLAGS} ${LDFLAGS} \
        -std=c++23 \
        -shared -o libxdpe1x2xx.so \
                    xdpe1x2xx_config.o xdpe1x2xx_firmware_dump.o \
                    xdpe1x2xx_utility.o xdpe1x2xx_controller_intf.o
    
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_dump.cpp \
        -std=c++23 \
        -L. \
        -lxdpe1x2xx \
        -li2c \
        -o xdpe1x2xx_dump
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        xdpe1x2xx_update.cpp \
        -std=c++23 \
        -L. \
        -lxdpe1x2xx \
        -li2c \
        -o xdpe1x2xx_update
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        test_xdpe1x2xx_dump.cpp \
        -std=c++23 \
        -L. \
        -lxdpe1x2xx \
        -li2c \
        -o test_xdpe1x2xx_dump
    ${CXX} ${CFLAGS} ${LDFLAGS} \
        test_xdpe1x2xx_update.cpp \
        -std=c++23 \
        -L. \
        -lxdpe1x2xx \
        -li2c \
        -o test_xdpe1x2xx_update
}

do_install() {
    install -d ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/xdpe1x2xx_dump ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/test_xdpe1x2xx_dump ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/xdpe1x2xx_update ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/test_xdpe1x2xx_update ${D}${base_bindir}
    install -d ${D}/${libdir}/${PN}
    install -m 0755 ${UNPACKDIR}/libxdpe1x2xx.so ${D}/${libdir}/${PN}
}
