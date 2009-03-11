ifeq ($(DEBUG),y)
	DBG_FLAGS = -g -DENABLE_DEBUG
else
	DBG_FLAGS =
endif

DBG_FLAGS += -DDEBUG_MOD_NAME=$(MOD_NAME)

EXTRA_CFLAGS += $(DBG_FLAGS)
EXTRA_CFLAGS += -I$(LDDINC)

MOD_OBJ = $(MOD_SRC:%.c=%.o)

ifneq ($(KERNELRELEASE),)
	$(MOD_NAME)-objs := $(MOD_OBJ)
	obj-m := $(MOD_NAME).o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	echo $(MOD_OBJ)
	$(MAKE) -C $(KERNELDIR) M=$(PWD)/ LDDINC=$(PWD)/../include modules

endif

clean:
	rm -rf *.ko *.o *.mod.c .*.cmd .tmp_versions Module.symvers modules.order

