obj-m := dms_led.o

ifdef ARCH
	KDIR := /work/linux
else
	KDIR := /lib/modules/$(shell uname -r)/build
endif

.PHONY: all led led-dev led-dts clean clean-all install

all: led

led: led-dev led-dts

# Build kernel module
led-dev:
	@if [ -f ./dms_led.c ] ; then \
		$(MAKE) -C $(KDIR) M=$(PWD) modules ;\
	else \
		echo "No dms_led.c found, skipping module build"; \
	fi

# Build device tree overlay
led-dts:
	@echo "Building device tree overlay..."
	@if [ -f ./dms_led.dts ] ; then \
		dtc -@ -I dts -O dtb -o dms_led.dtbo dms_led.dts ;\
	else \
		echo "No dms_led.dts found, skipping dtbo build"; \
	fi

# Delete file except .ko and .dtbo
clean:
	@rm -rf .tmp_versions *.mod.c *.o *.symvers *.mod modules.order
	@echo "Cleaned build files (except .ko and .dtbo)"

# Delete all file
clean-all: clean
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -f dms_led.dtbo
	@echo "Cleaned build files"

# install (dtbo + ko + udev + config.txt)
install: all
	@echo "[1] Copying dtbo to /boot/overlays/"
	@sudo cp dms_led.dtbo /boot/overlays/
	
	@echo "[2] Copying ko to kernel module path"
	@sudo mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/dms
	@sudo cp dms_led.ko /lib/modules/$(shell uname -r)/kernel/drivers/dms/
	@sudo depmod

	@echo "[3] Enabling overlay and module autoload"
	@sudo bash -c 'echo "dtoverlay=dms_led" >> /boot/firmware/config.txt'
	@echo "dms_led" | sudo tee /etc/modules-load.d/dms_led.conf

	@echo "[4] Installing udev rule for /dev/dms_led"
	@echo 'KERNEL=="dms_led", MODE="0666"' | sudo tee /etc/udev/rules.d/99-dms_led.rules > /dev/null
	@sudo udevadm control --reload
	@sudo udevadm trigger
	
	@echo "[✓] Installed: will auto-load after reboot"