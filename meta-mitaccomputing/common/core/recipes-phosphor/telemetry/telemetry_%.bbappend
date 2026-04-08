FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Make-the-AppendLimit-of-MetricReportDefinition-for-M.patch \
    "
