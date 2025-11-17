FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " bash"
RDEPENDS:${PN}:append = " mitac-common-functions"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.Capri_V2 \
    "

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}${libexecdir}/${PN}/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/mainboard-init-functions ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/thermal-mgmt-init-functions ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/mainboard_env.sh ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/spd_link.sh ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/fake_fru_Capri_V2.bin ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/update_fake_capri_fru.sh ${D}${libexecdir}/${PN}/${profile_name}/
        install -m 0644 ${UNPACKDIR}/${profile_name}/MRD*.json ${D}${libexecdir}/${PN}/${profile_name}/
    done
}
