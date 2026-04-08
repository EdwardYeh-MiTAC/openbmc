PREFERRED_PROVIDER_virtual/bootloader ?= "u-boot-aspeed-sdk"
PREFERRED_PROVIDER_u-boot ?= "u-boot-aspeed-sdk"
PREFERRED_VERSION_u-boot-aspeed-sdk = "v2023.10+%"
PREFERRED_PROVIDER_u-boot-fw-utils ?= "u-boot-fw-utils-aspeed-sdk"
#PREFERRED_VERSION_u-boot-fw-utils-aspeed-sdk = "v2019.04+%"
PREFERRED_VERSION_u-boot-fw-utils-aspeed-sdk = "v2023.10+%"
PREFERRED_PROVIDER_virtual/kernel ?= "linux-aspeed"
#PREFERRED_VERSION_linux-aspeed = "6.6+git"
PREFERRED_VERSION_linux-aspeed = "6.12+git"

# KBUILD_DEFCONFIG = "aspeed_g6_defconfig"

KERNEL_DEVICETREE = "aspeed/${KMACHINE}-bmc-mitac-base.dtb"
