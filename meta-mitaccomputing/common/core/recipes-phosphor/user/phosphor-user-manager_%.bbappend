FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRCREV = "9ca8692dedc633c6d2857f0a8dda8a3437ed50f9"
SRC_URI:append = " \
    file://0001-Fixed-user-enabled-state-is-abnormal-when-renaming.patch \
    "
