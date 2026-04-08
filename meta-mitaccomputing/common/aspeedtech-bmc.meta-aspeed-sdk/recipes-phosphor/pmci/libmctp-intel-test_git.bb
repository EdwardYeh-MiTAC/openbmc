SUMMARY = "Test application for libmctp-intel stack"
DESCRIPTION = "Test application for libmctp-intel stack"
PR = "r0"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"
SRC_URI = " file://mctp-astpcie-test.c;subdir=${BPN} \
            file://mctp-astpcie-test.h;subdir=${BPN} \
            file://mctp-smbus-test.c;subdir=${BPN} \
            file://mctp-smbus-test.h;subdir=${BPN} \
            file://mctp-test-utils.c;subdir=${BPN} \
            file://mctp-test-utils.h;subdir=${BPN} \
            file://CMakeLists.txt;subdir=${BPN} \
          "

DEPENDS = "libmctp-intel"
S = "${WORKDIR}/${BPN}"

inherit cmake
