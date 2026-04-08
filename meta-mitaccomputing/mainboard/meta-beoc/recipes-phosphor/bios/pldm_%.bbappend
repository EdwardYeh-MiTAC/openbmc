FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.S8050/ \
    file://com.mitac.Hardware.Chassis.Model.S8056/ \
    file://com.mitac.Hardware.Chassis.Model.S8261/ \
    file://com.mitac.Hardware.Chassis.Model.SC513G6/ \
    "

do_install:append() {
    install -m 0644 ${UNPACKDIR}/bios_attrs.json ${D}/usr/share/pldm/bios/bios_attrs.json
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}/usr/share/pldm/pdr/${profile_name}
        install -d ${D}/usr/share/pldm/bios/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/bios_attrs.json ${D}/usr/share/pldm/bios/${profile_name}/bios_attrs.json
    done
}
