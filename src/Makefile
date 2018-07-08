obj-m += tickle.o
tickle-objs := tickle_entry.o tickle_service.o tickle_io.o tickle_client.o tickle_usb.o tickle_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
