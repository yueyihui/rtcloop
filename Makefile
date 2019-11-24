obj-m+=rtc_loop.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ DBGFLAGS=-DDBG M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	find -type f -executable -delete
	find -type f -name *.o -delete
