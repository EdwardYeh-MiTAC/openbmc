FILESEXTRAPATHS:prepend:aspeed-g6 = "${THISDIR}/${PN}/AST2600:"
FILESEXTRAPATHS:prepend:aspeed-g7 = "${THISDIR}/${PN}/AST2700:"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

RDEPENDS:${PN} += " bash"
RDEPENDS:${PN} += " libgpiod-tools"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SRC_URI = " \
    file://fru-simple-read.c \
    file://mitac-common-functions \
    "

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} fru-simple-read.c -o fru-simple-read
}

do_install() {
    install -d ${D}${libexecdir}
    install -m 0755 ${UNPACKDIR}/mitac-common-functions ${D}${libexecdir}

    install -d ${D}${base_bindir}
    install -m 0755 ${UNPACKDIR}/fru-simple-read   ${D}${base_bindir}
}
