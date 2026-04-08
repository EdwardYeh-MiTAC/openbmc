FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
	install -d ${D}${datadir}/${PN}/${profile_name}
        install -m 0644 ${WORKDIR}/${profile_name}/power-config-host0.json ${D}${datadir}/${PN}/${profile_name}/
    done
}
