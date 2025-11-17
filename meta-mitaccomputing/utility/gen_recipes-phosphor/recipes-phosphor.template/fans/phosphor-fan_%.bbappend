FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " json"

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -d ${D}${datadir}/${PN}/${profile_name}
        install -m 0644 ${UNPACKDIR}/${profile_name}/*.json ${D}${datadir}/${PN}/${profile_name}/
    done
}
