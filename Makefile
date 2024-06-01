obj-m += tipi_gpio.o

all: tipi_gpio.ko tipi_rpi.dtbo 
potato: tipi_gpio.ko tipi_le_potato.dtbo

tipi_gpio.ko: tipi_gpio.c tipi_protocol.h
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(shell pwd) modules

%.pre.dts: %.dts
	$(CC) -E -nostdinc -Ilibretech-wiring-tool/include -x assembler-with-cpp -undef -o $@ $^

%.dtbo: %.pre.dts
	dtc -@ -I dts -O dtb -o $@ $<

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -f *.dtbo *.pre.dts
