#include "gps.h"

GPS::GPS(const std::string& device, int baudrate)
    : device(device), baudrate(baudrate), fd(-1) 
{
    if(!init()) {
        std::cerr << "Failed to initialize GPS on device: " << device << std::endl;
    }
}

bool GPS::init() {
    fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return false;
    }

    //termios 설정을 위한 baudrate 변환
    speed_t baud_rate;
    switch(baudrate) {
    case 9600: baud_rate = B9600; break;
    case 19200: baud_rate = B19200; break;
    case 115200: baud_rate = B115200; break;
    default: 
        std::cerr << "Unsupported baud rate: " << baudrate << std::endl;
        close(fd);
        return false;
    }
    
    //termios 설정
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    cfmakeraw(&options);

    if(tcsetattr(fd, TCSANOW, &options) < 0){
        perror("tcsetattr");
        return false;
    }
    return true;
}

bool GPS::update() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval timeout = {1, 0};
    int ret = select(fd + 1, &readfds, nullptr, nullptr, &timeout);
    if (ret > 0 && FD_ISSET(fd, &readfds)) {
        std::string buffer;
        buffer.resize(256);
        int n = read(fd, &buffer[0], buffer.size());
        if (n > 0) {
            buffer.resize(n);
            std::stringstream ss(buffer);
            std::string line;
            while (std::getline(ss, line)) {
                if (parseNMEA(line)) {
                    return true;
                }
            }
        }
    }
    else if (ret < 0) {
        perror("select");
    }
    return false;
}

// 각 필드값 저장하는 함수
void splitCSV(const std::string& line, std::vector<std::string>& fields)
{
    std::stringstream ss(line);
    std::string token;

    while(std::getline(ss, token, ',')){
        fields.push_back(token);
    }
}

double GPS::convertToDecimal(const std::string& raw, char direction) {
    double val = std::atof(raw.c_str());
    double degrees = std::floor(val / 100);
    double minutes = val - (degrees * 100);
    double decimal = degrees + (minutes / 60.0);
    if(direction == 'S' || direction == 'W') decimal = -decimal;
    return decimal; 
}

bool GPS::parseNMEA(const std::string& sentence) {
    // 지원 문장인지 검사
    if (!(sentence.find("$GPRMC") == 0 || sentence.find("$GPGGA") == 0 ||
        sentence.find("$GNRMC") == 0 || sentence.find("$GNGGA") == 0 ||
        sentence.find("$GNGLL") == 0 || sentence.find("$GPGLL") == 0)) {
        return false;
    }

    std::vector<std::string> fields;
    splitCSV(sentence, fields);

    int latIdx, latDirIdx, lonIdx, lonDirIdx;
    if(fields[0].find("GGA") != std::string::npos){
        latIdx = 2;
        latDirIdx = 3;
        lonIdx = 4;
        lonDirIdx = 5;
    }
    else if(fields[0].find("RMC") != std::string::npos){
        latIdx = 3;
        latDirIdx = 4;
        lonIdx = 5;
        lonDirIdx = 6;
    }
    else if(fields[0].find("GLL") != std::string::npos){
        latIdx = 1; 
        latDirIdx = 2; 
        lonIdx = 3; 
        lonDirIdx = 4;
    }
    else{
        return false;
    }

    // 필드 개수가 우리가 필요로 하는 인덱스보다 적으면 실패
    if(fields.size() <= std::max(std::max(latIdx, latDirIdx),
                       std::max(lonIdx, lonDirIdx)))
    {
        return false;
    }

    latitude = convertToDecimal(fields[latIdx], fields[latDirIdx][0]);
    longitude = convertToDecimal(fields[lonIdx], fields[lonDirIdx][0]);

    return true;
}

GPS::~GPS() {
    if (fd >= 0) close(fd);
}