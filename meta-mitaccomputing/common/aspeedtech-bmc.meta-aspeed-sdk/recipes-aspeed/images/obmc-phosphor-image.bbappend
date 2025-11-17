FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

IMAGE_CLASSES:append:aspeed-g7 = " image_types_phosphor_aspeed_g7"

# We use direct-with-blksz.py to create WIC file for UFS.
# Do not generate scripts/lib/wic/plugins/imager/__pycache__/
IMAGE_CMD:wic:prepend:ast-ufs () {
    export PYTHONDONTWRITEBYTECODE="1"
}
