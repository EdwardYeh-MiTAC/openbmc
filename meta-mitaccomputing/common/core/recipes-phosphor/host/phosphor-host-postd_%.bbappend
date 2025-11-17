FILESEXTRAPATHS:prepend := "${THISDIR}/phosphor-host-postd:"
RDEPENDS:${PN} += "bash"

SNOOP_DEVICE = "mitac-postcode-dev"
POST_CODE_BYTES = "4"

SRC_URI:append = "\
    file://0001-Using-shell-script-to-launch-snoopd-for-flexibility.patch \
    file://launchMitacPostCodeService.sh \
    "

do_install:append() {
    install -d ${D}/usr/libexec/
    install -m 0755 ${UNPACKDIR}/launchMitacPostCodeService.sh ${D}/usr/libexec/
}
