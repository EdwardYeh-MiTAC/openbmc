FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/common:"
FILESEXTRAPATHS:prepend:aspeed-g6 := "${THISDIR}/${PN}/AST2600:"
FILESEXTRAPATHS:prepend:aspeed-g7 := "${THISDIR}/${PN}/AST2700:"

# Mitac Common Configurations
SRC_URI:append = "\
    file://MiOBMC-common.cfg \
    file://mctp.cfg \
    file://aspeed-bmc-mitac-input-event-code.dtsi \
    file://MiOBMC-flash-layout-64.dtsi \
    file://MiOBMC-flash-layout-256-intel-pfr.dtsi \
    "
# It shall not activate CONFIG_MCTP_TRANSPORT_PCIE_VDM when enabled intel-legacy-mctp-pcie.
SRC_URI:append = " ${@bb.utils.contains("DISTRO_FEATURES", "intel-legacy-mctp-pcie", \
    "", \
    "file://mctp_pcie_vdm.cfg", \
    d)}"

# Mitac Common Configurations for AST2600
SRC_URI:append:aspeed-g6 = "\
    file://aspeed-bmc-mitac-common.dtsi \
    file://aspeed-bmc-mitac-common-leds.dtsi \
    file://aspeed-bmc-mitac-common-buttons.dtsi \
    file://aspeed-bmc-mitac-2600-enable-dp.dtsi \
    file://aspeed-bmc-mitac-2600-reserved-memory.dtsi \
    "

# Mitac Common Configurations for AST2700
SRC_URI:append:aspeed-g7 = "\
    file://aspeed-bmc-mitac-2700-common.dtsi \
    file://aspeed-bmc-mitac-2700-common-node2.dtsi \
    file://aspeed-bmc-mitac-2600-enable-dp.dtsi \
    file://aspeed-bmc-mitac-2700-reserved-memory.dtsi \
    "

SRC_URI:append:aspeed-g7 = "\
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
    cp ${UNPACKDIR}/MiOBMC-flash-layout-256-intel-pfr.dtsi ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/${KMACHINE}/
}
