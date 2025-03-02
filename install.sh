#!/bin/bash

if [ "x$(whoami)" != "xroot" ]; then
  echo "Error, must run as root. Aborting."
  exit 1
fi

if [ ! -f tipi_gpio.ko ]; then
  echo "Error no tipi_gpio.ko built. Aborting."
  exit 1
fi

mkdir -p /lib/modules/`uname -r`/kernel/drivers/tipi
cp tipi_gpio.ko /lib/modules/`uname -r`/kernel/drivers/tipi/

grep tipi_gpio /etc/modules >/dev/null || echo tipi_gpio >> /etc/modules

echo "options tipi_gpio sig_delay=100" >/etc/modprobe.d/tipi_gpio.conf

# This location only works for Raspberry PI
cp tipi_rpi.dtbo /boot/overlays/tipi.dtbo

grep dtoverlay=tipi /boot/firmware/config.txt >/dev/null || echo "dtoverlay=tipi" >> /boot/firmware/config.txt

depmod

if [ "${1:x}" = "/r" ]; then
  echo "Reloading tipi_gpio.ko"
  insmod /lib/modules/`uname -r`/kernel/drivers/tipi/tipi_gpio.ko
fi
