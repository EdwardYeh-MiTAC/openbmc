FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Added-the-command-for-smbios_blob_transfer-to-allowl.patch \
    file://0002-Added-dcmi-related-commands-to-allowlist.patch \
    "
