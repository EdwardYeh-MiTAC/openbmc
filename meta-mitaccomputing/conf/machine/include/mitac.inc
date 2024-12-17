require conf/machine/include/mitac-linux.inc

MACHINE_FEATURES += "\
        obmc-bmc-state-mgmt \
        obmc-chassis-state-mgmt \
        obmc-host-ipmi \
        obmc-host-state-mgmt \
        obmc-phosphor-chassis-mgmt \
        obmc-phosphor-fan-mgmt \
        obmc-phosphor-flash-mgmt \
        bonding \
        "

# OBMC Phosphor
IMAGE_INSTALL:append = "phosphor-led-sysfs elfutils phosphor-inventory-manager phosphor-nvme "
# Mitac Common
IMAGE_INSTALL:append = "mitac-common-functions plat-svc id-button "
# Networking
IMAGE_INSTALL:append = "ifenslave usb-network update-bond-mac "
# AMD Common
IMAGE_INSTALL:append = " ${@bb.utils.contains("DISTRO_FEATURES", "amd-common", "packagegroup-amd-common ", "", d)}"
# Remove recipes in amdese.meta-amd due to some components are in private repository.
BBMASK:append = " ${@bb.utils.contains("DISTRO_FEATURES", "amd-common", "", "/meta-mitaccomputing/common/amdese.meta-amd/", d)}"

SERIAL_CONSOLES = "115200;ttyS4"

VIRTUAL-RUNTIME_obmc-inventory-manager = "entity-manager"
PREFERRED_PROVIDER_virtual/obmc-inventory-data = "entity-manager"

VIRTUAL-RUNTIME_obmc-host-state-manager ?= "x86-power-control"
VIRTUAL-RUNTIME_obmc-chassis-state-manager ?= "x86-power-control"

PREFERRED_PROVIDER_virtual/obmc-chassis-mgmt = "packagegroup-mitac-apps"
PREFERRED_PROVIDER_virtual/obmc-flash-mgmt = "packagegroup-mitac-apps"
PREFERRED_PROVIDER_virtual/obmc-host-ipmi-hw ?= "phosphor-ipmi-kcs"
PREFERRED_PROVIDER_virtual/obmc-system-mgmt = "packagegroup-mitac-apps"
PREFERRED_PROVIDER_virtual/obmc-fan-mgmt = "packagegroup-mitac-apps"
