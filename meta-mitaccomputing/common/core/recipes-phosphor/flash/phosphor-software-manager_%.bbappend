FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://bios-update.sh \
    file://0001-Workaround-the-system-hang-issue-after-BIOS-update.patch \
    file://0002-Enable-bios-firmware-udpate-using-flashcp-utility.patch \
    file://0003-Temporarily-use-Other-to-identify-the-host-image-in-.patch \
    "

PACKAGECONFIG:append = " bios-software-update"
PACKAGECONFIG:append = " flash_bios"
PACKAGECONFIG:append = " i2cvr-software-update"

EXTRA_OEMESON:append = " -Dactive-bmc-max-allowed=2"

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    install -d ${D}/${sbindir}
    install -m 0755 ${UNPACKDIR}/bios-update.sh ${D}/${sbindir}/
}
