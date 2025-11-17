FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
PACKAGECONFIG:append = " transport-mctp-demux system-specific-bios-json"


SRC_URI:append = "\
    file://bios_attrs.json \
    file://host_eid \
    "

do_install:append() {
    install -d ${D}/usr/share/pldm/bios
    install -m 0644 ${UNPACKDIR}/bios_attrs.json ${D}/usr/share/pldm/bios/bios_attrs.json
    install -m 0644 ${UNPACKDIR}/host_eid ${D}/usr/share/pldm/host_eid
}
