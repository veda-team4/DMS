#ifndef BLUETOOTH_H
#define BLUETOOTH_H

class Bluetooth {
public:
  Bluetooth();
  ~Bluetooth();
  
private:
int fd;
char msg[10] = "RX\n";
char buf[10] = {0};
void Init();
void Close();
void HandleRx();
void TxFunc();
void RxFunc();

int is_rfcomm_bound();
int bind_rfcomm();

public:
void Motor();
};

#endif