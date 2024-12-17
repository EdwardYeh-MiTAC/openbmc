SUMMARY = "AMD CPU Info"
DESCRIPTION = "CPU Info monitors the dbus interface\
xyz.openbmc_project.Inventory.Item.Cpu_info.service for Processor property \
and applies the CPU values to the SOC using esmi oob library API's"

SRC_URI = "git://git@github.com/AMDESE/bmc-cpuinfo;branch=main;protocol=https"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

LICENSE = "CLOSED"
inherit obmc-phosphor-systemd
inherit cmake pkgconfig

def get_service(d):
      return "xyz.openbmc_project.Inventory.Item.Cpu_info.service"

SYSTEMD_SERVICE:${PN} = "${@get_service(d)}"

SRC_URI += "file://0001-Fixed-build-issue-by-upgrading-to-C-23.patch \
           "

DEPENDS += " \
    amd-apml \
    i2c-tools \
    libgpiod \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    "

RDEPENDS:${PN} += "amd-apml"

FILES:${PN} += "/usr/lib/systemd/system/xyz.openbmc_project.Inventory.Item.Cpu_info.service"
