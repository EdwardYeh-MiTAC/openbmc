KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.173"

# Tag for v00.04.19
SRCREV = "2f214b2be642e8beb411f7844a95b964bddcbf6d"

require linux-aspeed.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

DEPENDS += "lzop-native"
DEPENDS += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-secure-config-native', '', d)}"

SRC_URI:append = " file://ipmi_ssif.cfg "
SRC_URI:append = " file://mtd_test.cfg "
SRC_URI:append = " file://init_disassemble_info-signature-changes-causes-compile-failures.patch "
