# Linux Kernel driver for TIPI gpio 

## tipi_gpio.ko

Creates 2 character device files:

- /dev/tipi_control
- /dev/tipi_data

These represent the 4 registers held in the TIPI CPLD. 

| device            | Operation | Register |
| ----------------- | --------- | -------- |
| /dev/tipi_control | Read      |       TC |
| /dev/tipi_control | Write     |       RC |
| /dev/tipi_data    | Read      |       TD |
| /dev/tipi_data    | Write     |       RD |

Each platform supported must define a device-tree overlay 
that defines the tipi GPIO pins. The kernel module refers
to the declarations in the overlay to access the correct
pins by name, instead of hard-coding to the device. 

This should allow creation of alternative `tipi_gpio.dts` 
overlays that allow the same kernel module code to work 
on a variety of Linux capable single board computers.

## Raspberry PI

First draft, get this working on Raspberry PI OS (Bullseye)

- Install kernel headers
  - `sudo apt-get install raspberrypi-kernel-headers`

## Other boards

...

