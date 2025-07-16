#ifndef LED_H
#define LED_H
#define DMS_MAGIC  'l'
#define DMS_MAXNR   6

#define DMS_LED_OFF _IO(DMS_MAGIC, 0)
#define DMS_LED_ON  _IO(DMS_MAGIC, 1)

class Led {
public:
  Led();
  ~Led();

private:
  int fd;

public:
  void led_on();
  void led_off();
};

#endif