FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd systemd

RDEPENDS:${PN}:append = " bash"

SRC_URI:append = " \
    file://com.mitac.Hardware.Chassis.Model.Template/ \
    "

do_install:append() {
    for profile_name in ${PLATFORM_PROFILES}; do
        RESOURCE_DIR=${UNPACKDIR}/${profile_name}
        if [ "$(ls -A $RESOURCE_DIR)" ]; then
            install -m 0644 ${RESOURCE_DIR}/*.json ${D}${datadir}/${PN}/configurations/
        else
            echo "WARNING: ${RESOURCE_DIR} is Empty"
        fi
    done
}

