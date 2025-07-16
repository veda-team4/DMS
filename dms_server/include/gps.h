#ifndef GPS_H
#define GPS_H
#include <string>

class GPS {
public:
    GPS(const std::string& device = "/dev/ttyS0", int baudrate = 9600);
    ~GPS();

    bool cur_location(double* _latitute, double* _longitude);

private:
    int fd;
    int baudrate;
    std::string device;
    double latitude, longitude;

    bool init();
    bool update();
    bool parseNMEA(const std::string& sentence);
    double convertToDecimal(const std::string& raw, char direction);
};

#endif