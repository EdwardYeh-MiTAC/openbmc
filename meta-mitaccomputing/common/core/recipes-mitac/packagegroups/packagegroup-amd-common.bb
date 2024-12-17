SUMMARY = "Package group to support AMD platforms."

inherit packagegroup

RDEPENDS:${PN} = " \
    i2c-tools \
    amd-apml \
    amd-dimm \
    amd-ras  \
    cpu-info \
    i3c-tools \
    power-capping \
"
