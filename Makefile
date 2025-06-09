obj-m += tipi_pibus.o

all: tipi_pibus.ko tipi_pibus.dtbo 

tipi_pibus.ko: tipi_pibus.c tipi_protocol.h
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(shell pwd) modules

%.pre.dts: %.dts
	$(CC) -E -nostdinc -x assembler-with-cpp -undef -o $@ $^

%.dtbo: %.pre.dts
	dtc -@ -I dts -O dtb -o $@ $<

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -f *.dtbo *.pre.dts
