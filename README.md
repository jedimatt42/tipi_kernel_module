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

The tipi-reset pin is exported to the sysfs subsystem. 

see https://www.kernel.org/doc/Documentation/gpio/sysfs.txt

It is exported to: `/sys/devices/platform/tipi_gpio/tipi-reset`

Monitoring the reset signal can be handling by `poll` after 
configuring edge detection to `falling`.

## Params

sig_delay: default value 50 - If the communcation over the 
  wires from the SBC to the TIPI adapter is not reliable
  increase this busy loop counter for more signal propagation
  and settle time.

Configure the param by adding the following line to file 
/etc/modprobe.d/tipi_gpio.conf:

```
options tipi_gpio sig_delay=100
```

## Raspberry PI

First draft, get this working on Raspberry PI OS (Bullseye)

- Install kernel headers
  - `sudo apt-get install raspberrypi-kernel-headers`

- Compile the kernel module and device-tree overlay
  - `make`

- Install the kernel module and device-tree overlay
  - `sudo cp tipi_rpi.dtbo /boot/overlays/tipi.dtbo`
  - (notice the file rename to `tipi.dtbo`)
  - edit /boot/config.txt and add:
    - `dtoverlay=tipi`

## Other boards

### Le Potato

... should be able to make this work... 

### Renegade

... also maybe... some of the GPIO is wired oddly.

### Mango PI MQ Pro ( Allwinner D1 RISC-V core )

... maybe

## References

- https://www.kernel.org/doc/Documentation/devicetree/bindings/gpio/gpio.txt
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial

