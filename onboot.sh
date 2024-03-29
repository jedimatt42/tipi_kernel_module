#!/bin/bash

if [ ! -e /dev/tipi_control ]; then
  cd /home/tipi/tipi_kernel_module
  su tipi -c "make clean"
  su tipi -c make
  ./install.sh
  insmod /lib/modules/`uname -r`/kernel/drivers/tipi/tipi_gpio.ko
fi

