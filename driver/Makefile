# Makefile for building the filelogger kernel module
#
# Usage:
#   make          -> builds filelogger.ko
#   make clean    -> removes build files
#   sudo insmod filelogger.ko   -> load the driver
#   sudo rmmod filelogger       -> unload the driver
#   dmesg | tail                -> view kernel log output

obj-m += filelogger.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
