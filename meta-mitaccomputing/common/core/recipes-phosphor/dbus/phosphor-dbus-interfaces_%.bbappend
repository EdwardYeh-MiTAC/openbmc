FILESEXTRAPATHS:prepend := "${THISDIR}/phosphor-dbus-interfaces:"

S = "${WORKDIR}/git"

SRC_URI += "file://0001-com.amd-AMD-OEM-RAS-configuration-interface-added.patch \
            file://0001-Update-dbus-interface-definitions-to-support-MCTP-PC.patch \
            file://0002-Add-crashdump-interface.patch \
            "

EXTRA_OEMESON += "-Ddata_com_amd=true"

do_configure:prepend() {
  cd ${S}/gen && ./regenerate-meson
}