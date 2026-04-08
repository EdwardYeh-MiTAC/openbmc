# About porting

1. Modify pfr-mctp-i3c_git.bb to make pfr-mctp-i3c.service disabled by default.
2. The script mctp-i3c-starter.sh might not usable. It might moved the responsibility to platform's mainboard-init-functions.
3. Use gpiofind instead of magic number in scipt to make the script more portable.
