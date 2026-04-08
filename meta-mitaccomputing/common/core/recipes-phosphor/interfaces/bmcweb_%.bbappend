FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

EXTRA_OEMESON:append= " \
    -Dhttp-body-limit=264 \
    -Dredfish-dump-log=enabled \
    -Dredfish-host-logger=enabled \
    "

require bmcweb-boost_1.88.inc

SRC_URI:append = " \
    file://override.conf \
    "

# Add bmcweb service override to fix VM fail at Linux-6.12.
FILES:${PN} += "${systemd_system_unitdir}/bmcweb.service.d/override.conf"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/bmcweb.service.d
    install -m 0644 ${UNPACKDIR}/override.conf \
            ${D}${systemd_system_unitdir}/bmcweb.service.d/override.conf
}
