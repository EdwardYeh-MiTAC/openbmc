SUMMARY = "Dummy SMBPBI Gpu Manager"
DESCRIPTION = "Dummy SMBPBI Gpu Manager"
SECTION = "service"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILES:${PN} += "${bindir}/"

do_install () {
    install -d ${D}/${bindir}
    bbwarn "Run into dummy recipe. This build will not have this NDA module shipped."
    # bbnote "This build contains prebuilt binaries from NDA module."
}
