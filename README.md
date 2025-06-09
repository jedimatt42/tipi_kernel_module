# Linux Kernel driver for TIPI gpio 

## tipi_pibus.ko

This branch implements a 4 bit nibble transfer semi-qspi type 
protocol for interfacing with the TIPI controller board for
the Texas Instruments TI-99/4A. Requires corresponding firmware
on the TIPI board.

Creates 3 character device files:

- /dev/tipi_control
- /dev/tipi_data
- /dev/tipi_reset

These represent the 4 registers held in the TIPI CPLD, and the 
incoming reset signal.

| device            | Operation | Register |
| ----------------- | --------- | -------- |
| /dev/tipi_control | Read      |       TC |
| /dev/tipi_control | Write     |       RC |
| /dev/tipi_data    | Read      |       TD |
| /dev/tipi_data    | Write     |       RD |
| /dev/tipi_reset   | Poll      |          |

Each platform supported must define a device-tree overlay 
that defines the tipi GPIO pins. The kernel module refers
to the declarations in the overlay to access the correct
pins by name, instead of hard-coding to the device. 

This should allow creation of alternative `tipi_pibus.dts` 
overlays that allow the same kernel module code to work 
on a variety of Linux capable single board computers.

Monitoring the reset signal can be handling by `poll` for
POLLIN.

## Params

sig_delay: default value 50 - If the communcation over the 
  wires from the SBC to the TIPI adapter is not reliable
  increase this busy loop counter for more signal propagation
  and settle time.

Configure the param by adding the following line to file 
/etc/modprobe.d/tipi_pibus.conf:

```
options tipi_pibus sig_delay=100
```

## Raspberry PI

First draft, get this working on Raspberry PI OS (Bullseye)

- Install kernel headers
  - `sudo apt-get install raspberrypi-kernel-headers`

- Compile the kernel module and device-tree overlay
  - `make`

- Install the kernel module and device-tree overlay
  - `sudo cp tipi_pibus.dtbo /boot/overlays/tipi.dtbo`
  - (notice the file rename to `tipi.dtbo`)
  - edit /boot/config.txt and add:
    - `dtoverlay=tipi`

## References

- https://www.kernel.org/doc/Documentation/devicetree/bindings/gpio/gpio.txt
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial

