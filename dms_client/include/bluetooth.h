#ifndef BLUETOOTH_H
#define BLUETOOTH_H

class Bluetooth {
public:
  Bluetooth();
  ~Bluetooth();
  
private:
int fd;
char msg[10] = "RX\n";
char buf[30];
void Init();
void HandleTx();
void HandleRx();

public:
int tx_flag = 0;
void Close();
void TxFunc();
void RxFunc();
};

#endif