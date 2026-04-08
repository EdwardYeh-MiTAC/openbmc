FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://aspeed-bmc-mitac-base.dts \
    file://aspeed-bmc-mitac-s8050.dts \
    file://aspeed-bmc-mitac-s8056.dts \
    file://aspeed-bmc-mitac-s8261.dts \
    file://aspeed-bmc-mitac-sc513g6.dts \
    "

do_patch:append() {
  for DTB in ${KERNEL_DEVICETREE}; do
      DT=`/bin/basename $DTB .dtb`
      if [ -r "${WORKDIR}/${DT}.dts" ]; then
          cp ${WORKDIR}/${DT}.dts \
              ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/${KMACHINE}/
      fi
  done

}
