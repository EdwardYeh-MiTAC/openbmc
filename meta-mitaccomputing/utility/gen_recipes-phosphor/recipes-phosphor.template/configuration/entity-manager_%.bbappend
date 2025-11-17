FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

RDEPENDS:${PN}:append = " bash"

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        install -m 0644 ${UNPACKDIR}/${profile_name}/*.json ${D}${datadir}/${PN}/configurations/
    done
}
