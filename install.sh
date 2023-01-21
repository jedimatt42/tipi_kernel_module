#!/bin/bash

if [ ! -f tipi_gpio.ko ]; then
  echo "Error no tipi_gpio.ko built. Aborting."
  exit 1
fi

mkdir -p /lib/modules/`uname -r`/kernel/drivers/tipi
cp tipi_gpio.ko /lib/modules/`uname -r`/kernel/drivers/tipi/

grep tipi_gpio /etc/modules >/dev/null || echo tipi_gpio >> /etc/modules

echo "options tipi_gpio sig_delay=100" >/etc/modprobe.d/tipi_gpio.conf

depmod