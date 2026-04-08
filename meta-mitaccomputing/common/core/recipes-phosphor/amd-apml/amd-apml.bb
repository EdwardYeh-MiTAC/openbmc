SUMMARY = "AMD EPYC System Management Interface Library"
DESCRIPTION = "AMD EPYC System Management Interface Library for user space APML implementation"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

LICENSE = "CLOSED"

DEPENDS += "i2c-tools"
DEPENDS += "i3c-tools"
RDEPENDS:${PN} += "bash"

DEPENDS += "virtual/kernel"

SRC_URI += "git://github.com/amd/esmi_oob_library;protocol=https;branch=master"
SRCREV = "1c5f45363cbb7d39bf3344bc5d522956455b41fc"

S="${WORKDIR}/git"

inherit cmake

EXTRA_OEMAKE += " \
    KDIR=${STAGING_KERNEL_DIR} \
"

do_configure:prepend() {
    install -d ${STAGING_DIR_HOST}${includedir}/linux
    install -m 0755 ${STAGING_KERNEL_DIR}/include/uapi/linux/amd-apml.h ${STAGING_DIR_HOST}${includedir}/linux/amd-apml.h
}

do_install () {
        install -d ${D}${libdir}
        cp --preserve=mode,timestamps -R ${B}/libapml* ${D}${libdir}/

        install -d ${D}${bindir}
        install -m 0755 ${B}/apml_cpuid_tool ${D}${bindir}/
        install -m 0755 ${B}/apml_tool ${D}${bindir}/

        install -d ${D}${includedir}
        install -m 0644 ${S}/include/esmi_oob/* ${D}${includedir}/
}

FILES_${PN} += "${includedir}/linux/amd-apml.h"