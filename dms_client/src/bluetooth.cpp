#include "bluetooth.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>
#include <chrono>
#include <sys/select.h>

#define BT_DEVICE "/dev/rfcomm0"


Bluetooth::Bluetooth() {
    Init();
}
Bluetooth::~Bluetooth() {
    Close();
}


void Bluetooth::Init() {
    fd = open("/dev/rfcomm0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        throw std::runtime_error("Failed to open port\n");
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);    // 로컬 연결, 수신 가능 설정
    options.c_cflag &= ~PARENB;             // 패리티 없음
    options.c_cflag &= ~CSTOPB;             // 1 stop bit
    options.c_cflag &= ~CSIZE; 		        //비트 크기 초기화
    options.c_cflag |= CS8;                 // 8bit 데이터

    // options.c_cflag &= ~CRTSCTS;
    
    // options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // options.c_iflag &= ~(IXON | IXOFF | IXANY);
    // options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);	//설정 내용 즉시 적용

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void Bluetooth::Close() {
    close(fd);
}
void Bluetooth::HandleTx() {
    //if(fd==-1) return;
    write(fd, msg, strlen(msg));
    std::cout << "Tx : " << msg << std::endl;
}
void Bluetooth::HandleRx() {
    //if(fd==-1) return;
    int n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Rx : " << buf << std::endl;
    }
}
void Bluetooth::TxFunc() {
    if (tx_flag > 5) {
        tx_flag = 0;
        HandleTx();
    }
}
void Bluetooth::RxFunc() {
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(fd, &read_fds)) {
        HandleRx();
    }
}