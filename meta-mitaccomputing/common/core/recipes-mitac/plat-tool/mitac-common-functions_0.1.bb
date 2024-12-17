LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

RDEPENDS:${PN} += " bash"
RDEPENDS:${PN} += " libgpiod-tools"

S = "${WORKDIR}"

SRC_URI = " \
    file://fru-simple-read.c \
    file://soc-aspeed-init.c \
    file://mitac-common-functions \
    "

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} fru-simple-read.c -o fru-simple-read
    ${CC} ${CFLAGS} ${LDFLAGS} soc-aspeed-init.c -o soc-aspeed-init
}

do_install() {
    install -d ${D}${libexecdir}
    install -m 0755 ${WORKDIR}/mitac-common-functions ${D}${libexecdir}

    install -d ${D}${base_bindir}
    install -m 0755 ${WORKDIR}/fru-simple-read   ${D}${base_bindir}
    install -m 0755 ${WORKDIR}/soc-aspeed-init   ${D}${base_bindir}
}
