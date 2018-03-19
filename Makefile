MODULE_NAME = elevator

ifneq ($(KERNELRELEASE),)
	obj-y := elevator_calls.o
	obj-m := elevator.o
else
	KERNELDIR ?= /lib/modules/`uname -r`/build/
	PWD :=`pwd`
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
clean:
	rm -f *.ko *.o Module* *mod*
