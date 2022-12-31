#!/bin/bash

if [ ! -f tipi_gpio.ko ]; then
  echo "Error no tipi_gpio.ko built. Aborting."
  exit 1
fi

mkdir -p /lib/modules/`uname -r`/kernel/drivers/tipi
cp tipi_gpio.ko /lib/modules/`uname -r`/kernel/drivers/tipi/

grep tipi_gpio /etc/modules >/dev/null || echo tipi_gpio >> /etc/modules
depmod
