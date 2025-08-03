#!/bin/bash

if [ "x$(whoami)" != "xroot" ]; then
  echo "Error, must run as root. Aborting."
  exit 1
fi

if [ ! -f tipi_pibus.ko ]; then
  echo "Error no tipi_pibus.ko built. Aborting."
  exit 1
fi

mkdir -p /lib/modules/`uname -r`/kernel/drivers/tipi
cp tipi_pibus.ko /lib/modules/`uname -r`/kernel/drivers/tipi/

grep tipi_pibus /etc/modules >/dev/null || echo tipi_pibus >> /etc/modules

echo "options tipi_pibus sig_delay=100" >/etc/modprobe.d/tipi_pibus.conf

# This location only works for Raspberry PI
cp tipi_pibus.dtbo /boot/overlays/tipi.dtbo

grep dtoverlay=tipi /boot/firmware/config.txt >/dev/null || echo "dtoverlay=tipi" >> /boot/firmware/config.txt

depmod

if [ "${1:x}" = "/r" ]; then
  echo "Reloading tipi_pibus.ko"
  insmod /lib/modules/`uname -r`/kernel/drivers/tipi/tipi_pibus.ko
fi
