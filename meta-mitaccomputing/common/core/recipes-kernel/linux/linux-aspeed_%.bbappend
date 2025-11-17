FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = "\
    file://0001-Support-I3C-Hub.patch \
    file://0002-Fixed-build-issue-from-casting.patch \
    file://0003-To-improve-accustic-PWM-will-set-to-the-maximum-stat.patch \
    file://0004-Support-peci-legacy-device-in-device-t.patch \
    file://0005-Migrate-NCT7363-HWMON-driver-from-Linux-6.13.patch \
    file://0006-Fixed-dac_mux-issue-when-using-display-port-and-AST2.patch \
    file://0007-Fixed-build-issue-from-casting.patch \
    file://0008-AUXILIARY_BUS-is-mandatory-for-ASPEED_RESET.patch \
    file://0009-Add-AMD-APML-driver.patch \
    file://0010-Workaround-mctp-pcie-vdm-and-mctp-route-to-support-r.patch \
    file://0011-Optional-Beautify-dump-of-packet-data.patch \
    file://0012-Supported-BMC-as-MCTP-bus-owner-on-AMD-platforms-and.patch \
    "

# Mitac Common Configurations
SRC_URI:append = "\
    file://s8261.cfg \
    file://mctp.cfg \
    file://aspeed-bmc-mitac-common.dtsi \
    file://aspeed-bmc-mitac-common-leds.dtsi \
    file://aspeed-bmc-mitac-common-buttons.dtsi \
    file://aspeed-bmc-mitac-input-event-code.dtsi \
    file://MiOBMC-flash-layout-64.dtsi \
    "
# It shall not activate CONFIG_MCTP_TRANSPORT_PCIE_VDM when enabled intel-legacy-mctp-pcie.
SRC_URI:append = " ${@bb.utils.contains("DISTRO_FEATURES", "intel-legacy-mctp-pcie", \
    "", \
    "file://mctp_pcie_vdm.cfg", \
    d)}"


# Mitac Common Configurations for AST2600
SRC_URI:append = "\
    file://aspeed-bmc-mitac-2600-enable-dp.dtsi \
    file://aspeed-bmc-mitac-2600-reserved-memory.dtsi \
    "

# Mitac Common Configurations for AST2700
SRC_URI:append = "\
    file://aspeed-g7/ \
    file://mitac_2700.cfg \
    "

# Mitac Common Configurations for Intel BHS
SRC_URI:append = "\
    file://aspeed-bmc-mitac-common-intel.dtsi \
    file://aspeed-bmc-mitac-common-intel-bhs-sgpio.dtsi \
    file://aspeed-bmc-mitac-common-intel-bhs-vwgpio.dtsi \
    file://aspeed-bmc-mitac-common-intel-bhs-sgpio-leds.dtsi \
    "

do_patch:append() {
    for DTB in ${KERNEL_DEVICETREE}; do
        DT=`/bin/basename $DTB .dtb`
        if [ -r "${UNPACKDIR}/${DT}.dts" ]; then
            cp ${UNPACKDIR}/${DT}.dts \
                ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/${KMACHINE}/
        fi
    done
    cp ${UNPACKDIR}/aspeed-bmc-mitac*.dtsi ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/${KMACHINE}/
    cp ${UNPACKDIR}/MiOBMC-flash-layout-64.dtsi ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/${KMACHINE}/
}
