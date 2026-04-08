
* To check image size

```
jm@jm-pi2-desktop:~/spi/beoc/flashsize$ tar xvf beoc.obmc.20250717.tar
image-u-boot
image-kernel
image-rofs
image-rwfs
MANIFEST
publickey
image-u-boot.sig
image-kernel.sig
image-rofs.sig
image-rwfs.sig
MANIFEST.sig
publickey.sig
image-full.sig
jm@jm-pi2-desktop:~/spi/beoc/flashsize$ ls -l
total 78660
-rw-r--r-- 1 jm jm 40263680 Jul 18 09:16 beoc.obmc.20250717.tar
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 image-full.sig
-rw-r--r-- 1 jm jm  7038820 Jul 17 18:15 image-kernel
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 image-kernel.sig
-rw-r--r-- 1 jm jm 32477184 Jul 17 18:16 image-rofs
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 image-rofs.sig
-rw-r--r-- 1 jm jm        0 Jul 17 18:16 image-rwfs
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 image-rwfs.sig
-rw-r--r-- 1 jm jm   725172 Jul 17 18:16 image-u-boot
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 image-u-boot.sig
-rw-r--r-- 1 jm jm      273 Jul 17 18:16 MANIFEST
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 MANIFEST.sig
-rw-r--r-- 1 jm jm      800 Jul 17 18:16 publickey
-rw-r--r-- 1 jm jm      512 Jul 17 18:16 publickey.sig
```

* To evaluate storage usage for files deployed in image-rofs

```
ubuntu@5b201fc5ca73:/MIC/MiOBMC/beoc/openbmc/build/beoc/tmp/work/beoc-openbmc-linux-gnueabi/obmc-phosphor-image/1.0/rootfs$ find . -type f -size +500k -print0 | xargs -0 ls -lhS
-rwxr-xr-x 1 ubuntu ubuntu 4.0M Mar  9  2018 ./usr/libexec/bmcwebd
-rwxr-xr-x 1 ubuntu ubuntu 3.4M Mar  9  2018 ./usr/lib/systemd/libsystemd-shared-257.so
-rwxr-xr-x 1 ubuntu ubuntu 3.3M Mar  9  2018 ./usr/lib/libphosphor_dbus.so.1.0.0
-rwxr-xr-x 1 ubuntu ubuntu 3.1M Mar  9  2018 ./usr/lib/libcrypto.so.3
-rwxr-xr-x 1 ubuntu ubuntu 2.6M Mar  9  2018 ./usr/bin/phosphor-inventory
-rwxr-xr-x 1 ubuntu ubuntu 2.0M Mar  9  2018 ./usr/lib/libstdc++.so.6.0.33
-rwxr-xr-x 1 ubuntu ubuntu 2.0M Mar  9  2018 ./usr/lib/ipmid-providers/libzinteloemcmds.so.0.1
-rwxr-xr-x 1 ubuntu ubuntu 1.9M Mar  9  2018 ./usr/lib/systemd/libsystemd-core-257.so
-rwxr-xr-x 1 ubuntu ubuntu 1.8M Mar  9  2018 ./usr/libexec/phosphor-virtual-sensor/virtual-sensor
-rwxr-xr-x 1 ubuntu ubuntu 1.6M Mar  9  2018 ./usr/lib/systemd/systemd-networkd
-rwxr-xr-x 1 ubuntu ubuntu 1.4M Mar  9  2018 ./usr/lib/libc.so.6
-rwxr-xr-x 1 ubuntu ubuntu 1.4M Mar  9  2018 ./usr/lib/ipmid-providers/libipmi20.so.0.1
-rwxr-xr-x 1 ubuntu ubuntu 1.3M Mar  9  2018 ./usr/bin/udevadm
-rwxr-xr-x 1 ubuntu ubuntu 1.2M Mar  9  2018 ./usr/sbin/nvme
-rwxr-xr-x 1 ubuntu ubuntu 1.2M Mar  9  2018 ./usr/libexec/entity-manager/entity-manager
-rwxr-xr-x 1 ubuntu ubuntu 1.1M Mar  9  2018 ./usr/bin/bash.bash
-rwxr-xr-x 1 ubuntu ubuntu 1.1M Mar  9  2018 ./usr/bin/psusensor
-rwxr-xr-x 1 ubuntu ubuntu 960K Mar  9  2018 ./usr/lib/libsystemd.so.0.40.0
-rwxr-xr-x 1 ubuntu ubuntu 932K Mar  9  2018 ./usr/lib/ipmid-providers/libdynamiccmds.so.0.1
-rwxr-xr-x 1 ubuntu ubuntu 885K Mar  9  2018 ./usr/lib/libcairo.so.2.11804.4
-rwxr-xr-x 1 ubuntu ubuntu 881K Mar  9  2018 ./usr/lib/libpldmresponder.so.0.1
-rw-r--r-- 1 ubuntu ubuntu 874K Mar  9  2018 ./usr/share/www/css/app.735b449e.css.gz
-rwxr-xr-x 1 ubuntu ubuntu 839K Mar  9  2018 ./usr/bin/pldmd
-rwxr-xr-x 1 ubuntu ubuntu 829K Mar  9  2018 ./usr/bin/ipmitool
-rwxr-xr-x 1 ubuntu ubuntu 812K Mar  9  2018 ./usr/bin/swampd
-rwxr-xr-x 1 ubuntu ubuntu 799K Mar  9  2018 ./usr/bin/telemetry
-rwxr-xr-x 1 ubuntu ubuntu 756K Mar  9  2018 ./usr/lib/libssl.so.3
-rwxr-xr-x 1 ubuntu ubuntu 743K Mar  9  2018 ./usr/bin/phosphor-fan-control
-rwxr-xr-x 1 ubuntu ubuntu 733K Mar  9  2018 ./usr/bin/power-control
-rwxr-xr-x 1 ubuntu ubuntu 694K Mar  9  2018 ./usr/lib/libzstd.so.1.5.7
-rwxr-xr-x 1 ubuntu ubuntu 681K Mar  9  2018 ./usr/libexec/phosphor-code-mgmt/phosphor-bios-software-update
-rwxr-xr-x 1 ubuntu ubuntu 672K Mar  9  2018 ./usr/lib/libkrb5.so.3.3
-rwxr-xr-x 1 ubuntu ubuntu 671K Mar  9  2018 ./usr/sbin/rsyslogd
-rwxr-xr-x 1 ubuntu ubuntu 668K Mar  9  2018 ./usr/bin/intelcpusensor
-rwxr-xr-x 1 ubuntu ubuntu 656K Mar  9  2018 ./usr/bin/netipmid
-rwxr-xr-x 1 ubuntu ubuntu 650K Mar  9  2018 ./usr/bin/busybox.nosuid
-rwxr-xr-x 1 ubuntu ubuntu 650K Mar  9  2018 ./usr/bin/pldmtool
-rw-r--r-- 1 ubuntu ubuntu 627K Mar  9  2018 ./usr/share/www/js/app.46091b44.js.gz
-rwxr-xr-x 1 ubuntu ubuntu 612K Mar  9  2018 ./usr/bin/hwmontempsensor
-rwxr-xr-x 1 ubuntu ubuntu 608K Mar  9  2018 ./usr/bin/fansensor
-rwxr-xr-x 1 ubuntu ubuntu 586K Mar  9  2018 ./usr/lib/libfreetype.so.6.20.2
-rwxr-xr-x 1 ubuntu ubuntu 555K Mar  9  2018 ./usr/lib/libdw-0.192.so
-rwxr-xr-x 1 ubuntu ubuntu 540K Mar  9  2018 ./usr/bin/exitairtempsensor
-rwxr-xr-x 1 ubuntu ubuntu 532K Mar  9  2018 ./usr/bin/adcsensor
-rwxr-xr-x 1 ubuntu ubuntu 506K Mar  9  2018 ./usr/bin/smbiosmdrv2app
-rwxr-xr-x 1 ubuntu ubuntu 505K Mar  9  2018 ./usr/bin/phosphor-hwmon-readd

```
