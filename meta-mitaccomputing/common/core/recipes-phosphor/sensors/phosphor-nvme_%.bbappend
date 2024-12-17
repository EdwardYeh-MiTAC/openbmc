FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
inherit obmc-phosphor-systemd

SYSTEMD_SERVICE:${PN} = "xyz.openbmc_project.nvme.manager.service"

RDEPENDS:${PN} += "bash"

FILES:${PN}:append = " ${libexecdir}/mitac-nvme-init-main"


SRC_URI:append = " \
	file://mitac-nvme-init-main \
	file://0001-Moved-the-declaration-of-static-unordered_map-from-f.patch \
	"

do_install:append() {
    install -d ${D}${libexecdir}
    install -m 0755 ${WORKDIR}/mitac-nvme-init-main ${D}${libexecdir}
}
