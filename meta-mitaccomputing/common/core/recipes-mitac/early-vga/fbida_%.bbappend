FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
PACKAGECONFIG = "gif png"

DEPENDS:append= " dbus"
REQUIRED_DISTRO_FEATURES = ""

SRC_URI:append = "\
    file://0001-Update-meson-build-configuration-to-allow-the-suppor.patch \
    file://0002-Change-the-font-size-and-color-for-shadow_draw_strin.patch \
    file://0003-Copy-from-fbi.c-for-edit.patch \
    file://0004-Removed-unnecessary-function.patch \
    file://0005-Rewrite-early_vga-based-on-fbida-2.14-and-for-the-ch.patch \
    file://0006-Show-ethernet-interfaces-depending-on-presence-of-bo.patch \
    "
