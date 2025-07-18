#include "led.h"
#include <iostream>
#include <unistd.h> // close, usleep 등 시스템 호출
#include <fcntl.h> // for open 
#include <sys/ioctl.h> // for ioctl
#include <stdlib.h> // for exit

Led::Led() {
    // Open device file
    fd = open("/dev/dms_led", O_RDWR);

    if(fd < 0){
        std::cerr << "/dev/dms_led open error" << std::endl;
        std::cout << "[led] please check if /dev/dms_led exist" << std::endl;
        exit(-1);
    }
}

Led::~Led() {
    close(fd);
}

void Led::led_on() {
    ioctl(fd, DMS_LED_ON, 0);
}

void Led::led_off() {
    ioctl(fd, DMS_LED_OFF, 0);
}