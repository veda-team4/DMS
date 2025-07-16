#pragma once
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sstream>
#include <iostream>
#include <cmath>


class GPS {
public:
    GPS(const std::string& device = "/dev/ttyS0", int baudrate = 9600);
    ~GPS();

    bool init();
    bool update();
    std::pair<double, double> cur_location() const {
        return {latitude, longitude};
    }

private:
    int fd;
    int baudrate;
    std::string device;

    double latitude, longitude;

    bool parseNMEA(const std::string& sentence);
    double convertToDecimal(const std::string& raw, char direction);
};
