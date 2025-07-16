obj-m := dms_led.o

ifdef ARCH
	KDIR := /work/linux
else
	KDIR := /lib/modules/$(shell uname -r)/build
endif

.PHONY: all led led-dev led-app led-dts clean disp help

all: led

led: led-dev led-app led-dts disp

led-dev:
	@if [ -f ./dms_led.c ] ; then \
		$(MAKE) -C $(KDIR) M=$(PWD) modules ;\
	else \
		echo "No dms_led.c found, skipping module build"; \
	fi

led-dts:
	@echo "Building device tree overlay..."
	@if [ -f ./dms_led.dts ] ; then \
		dtc -@ -I dts -O dtb -o dms_led.dtbo dms_led.dts ;\
	else \
		echo "No dms_led.dts found, skipping dtbo build"; \
	fi

disp:
	@echo "=============================="
	@if [ -f ./dms_led.ko ] ; then \
		echo "insmod dms_led.ko"; \
	else \
		echo "No dms_led.ko found"; \
	fi
	@if [ -f ./dms_led.dtbo ] ; then \
		echo "dtoverlay dms_led.dtbo"; \
	else \
		echo "No dms_led.dtbo found"; \
	fi
	@echo "=============================="

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -rf .tmp_versions *.mod.c *.o *.symvers modules.order
	@echo "Cleaned build files"

clean-all: clean
	@rm -f dms_led.dtbo
	@echo "Also deleted dtbo"

install: all
	@echo "[1] Copying dtbo to /boot/overlays/"
	@sudo cp dms_led.dtbo /boot/overlays/
	
	@echo "[2] Copying ko to kernel module path"
	@sudo mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/dms
	@sudo cp dms_led.ko /lib/modules/$(shell uname -r)/kernel/drivers/dms/
	@sudo depmod

	@echo "[3] Enabling overlay and module autoload"
	@sudo bash -c 'echo "dtoverlay=dms_led" >> /boot/config.txt'
	@echo "dms_led" | sudo tee /etc/modules-load.d/dms_led.conf
	
	@echo "[✓] Installed: will auto-load after reboot"