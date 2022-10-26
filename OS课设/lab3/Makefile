ifneq ($(KERNELRELEASE),)
#kbuild syntax.
#模块的文件组成
#mymodule-objs :=mychrdriver.o   
#生成的模块文件名
obj-m := mychrdriver.o
else
PWD :=$(shell pwd)
KVER :=$(shell uname -r)
KDIR :=/lib/modules/$(KVER)/build
all:
	$(MAKE) -C $(KDIR) M=$(PWD)
clean:
#	rm -f *.cmd *.o *.mod *.ko
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions
#	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
