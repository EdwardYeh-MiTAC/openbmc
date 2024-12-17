PREFERRED_PROVIDER_virtual/kernel ?= "linux-aspeed"
PREFERRED_VERSION_linux-aspeed = "6.6.52+git"

KBUILD_DEFCONFIG = "aspeed_g6_defconfig"

KERNEL_DEVICETREE = "aspeed/${KMACHINE}-bmc-mitac-base.dtb"
