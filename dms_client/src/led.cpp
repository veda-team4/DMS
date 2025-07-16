#include "led.h"
#include <iostream>
#include <unistd.h> // close, usleep 등 시스템 호출
#include <fcntl.h> // for open 
#include <sys/ioctl.h> // for ioctl
#include <stdlib.h> // for exit

Led::Led() {
    std::cout << "[led] led constructor ~" << std::endl;
    // Open device file
    fd = open("/dev/dms_led", O_RDWR);
    std::cout << "fd = " << fd << std::endl;

    if(fd < 0){
        std::cerr << "/dev/dms_led open error !!!" << std::endl;
        std::cout << "[led] please check if /dev/dms_led exist !!!" << std::endl;
        exit(-1);
    }
}

Led::~Led() {
    close(fd);
    std::cout << "[led] led desturctor ~" << std::endl;
}

void Led::led_on() {
    ioctl(fd, DMS_LED_ON, 0);
    std::cout << "[led] led on !!" << std::endl;
}

void Led::led_off() {
    ioctl(fd, DMS_LED_OFF, 0);
    std::cout << "[led] led off !!" << std::endl;
}