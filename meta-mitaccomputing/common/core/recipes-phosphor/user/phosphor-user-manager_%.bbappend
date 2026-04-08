FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Fixed-user-enabled-state-is-abnormal-when-renaming.patch \
    "
