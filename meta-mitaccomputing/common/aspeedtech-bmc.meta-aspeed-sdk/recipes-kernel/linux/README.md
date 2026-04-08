MiOBMC use Linux 6.12 as default build configuration and also keep to support Linux 6.6 (6.6.106 specifically) in case of necessary.

For each migration of Aspeed SDK, it might needs to refine the SRCREV in linux-aspeed_6.6.bb to make sure it stay at the supported version.
You also needs to modify the PREFERRED_VERSION_linux-aspeed in mitac-aspeed-linux.inc to verify it could build with the preferred version of linux.
    ../../../../conf/machine/include/mitac-aspeed-linux.inc
