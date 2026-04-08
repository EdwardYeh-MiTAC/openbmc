SUMMARY = "AMD RAS application to handle RAS errors from BMC"
DESCRIPTION = "The applications harvests and handles the RAS errors from the processor"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

inherit meson pkgconfig systemd

SRC_URI = "git://git@github.com:/AMDESE/amd-bmc-ras.git;branch=integ_sp7;protocol=ssh"
SRCREV = "8df870faaaa72b06be671a595091efcd7e70d0d2"

S = "${WORKDIR}/git"

SRC_URI += "file://0001-Fix-build-error.patch \
            file://0002-Disable-auto-start-ras-service.patch \
            "

DEPENDS += " \
    amd-apml \
    libcper-header \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    libgpiod \
    boost \
    nlohmann-json \
    "

do_configure:prepend() {
    install -d ${STAGING_DIR_HOST}${includedir}/linux
    install -m 0755 ${STAGING_KERNEL_DIR}/include/uapi/linux/amd-apml.h ${STAGING_DIR_HOST}${includedir}/linux/amd-apml.h
}

SYSTEMD_SERVICE:${PN} = "com.amd.RAS.service"
FILES:${PN} += "${datadir}/amd-bmc-ras"
