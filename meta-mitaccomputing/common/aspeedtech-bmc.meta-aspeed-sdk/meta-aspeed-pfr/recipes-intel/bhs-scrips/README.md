# About porting

1. Modify bmc-boot-done.bb to make xyz.openbmc_project.bmc_boot_done.service disabled by default.
2. The script bmc-boot-done.sh might not usable. It might moved the responsibility to platform's mainboard-init-functions.
3. Use gpiofind instead of magic number in scipt to make the script more portable.
