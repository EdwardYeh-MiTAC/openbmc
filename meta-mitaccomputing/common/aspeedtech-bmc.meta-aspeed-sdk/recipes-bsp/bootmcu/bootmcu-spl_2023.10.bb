require bootmcu-spl.inc
require common/aspeedtech-bmc.meta-aspeed-sdk/recipes-bsp/u-boot/u-boot-common-aspeed-sdk_${PV}.inc

SRC_URI:append = " \
    file://0001-Fixed-build-issue-by-following-the-new-function-sign.patch \
    "
