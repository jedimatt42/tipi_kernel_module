# Linux Kernel driver for TIPI gpio 

## tipi_gpio.ko

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

This should allow creation of alternative `tipi_gpio.dts` 
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

Mostly works, just have to figure out how to enable the interrupt support
on the tipi-reset-gpio

I never figured out how to reconfigure interrupts for that pin. They are
all disabled by default on the Le Potato by default as there are only
a few interrupts available. On this device tipiwatchdog reads the input in
a hot loop, using up one of the 4 cores ono the device. 

2024-05-09 : This is broken again after updating away from the sysfs to a character dev
for the /dev/tipi_reset signal.

- `make` the kernel module. 
- install the tipi device tree overlay
  - `sudo cp tipi_le_potato.dtbo /opt/librecomputer/libretech-wiring-tool/libre-computer/aml-s905x-cc/dt/tipi.dtbo`
  - `sudo ldto enable tipi`
  - `sudo ldto merge tipi`
- load the kernel module
  - `sudo insmod tipi_gpio.ko`

### Renegade

... also maybe... 

### Mango PI MQ Pro ( Allwinner D1 RISC-V core )

... maybe

## References

- https://www.kernel.org/doc/Documentation/devicetree/bindings/gpio/gpio.txt
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial

