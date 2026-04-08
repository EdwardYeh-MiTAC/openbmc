FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = "\
    file://0001-Support-nvme-i-message-type-since-NVMe-Mi-message-re.patch \
    file://0002-mctpd-accept-set-endpoint-ID-as-endpoint-and-discove.patch \
    "
