# PACKAGECONFIG:append = " log-threshold log-pulse send-to-logger"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
	file://0001-Experimental-patch-to-support-Fatal-event.patch \
	"
