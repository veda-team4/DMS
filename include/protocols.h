#ifndef PROTOCOLS_H
#define PROTOCOLS_H

/*
통신 간에 사용할 프로토콜 정의

서버 -> 클라이언트 프로토콜은 아래와 같다
[Type(1byte)][Len(4byte)][Data]

첫 1바이트는 프로토콜 타입을,
이후 4바이트는 Data의 byte 크기를
이후에는 Data가 전송된다

서버 <- 클라이언트 프로토콜은 아래와 같다.
[Command(1byte)]

protocols.h 에는 첫 1바이트에 해당하는 Type 을 정의한다
*/

namespace Protocol {
  enum Type : uint8_t {
    // SERVER <- CLIENT [COMMAND]
    CAMSET,
    CALIBRATE,
    CALIBRATE_OPENED,
    CALIBRATE_CLOSED,
    CALIBRATE_FINISH,
    MONITOR,
    STOP,
    NOTHING,
    // SERVER -> CLIENT [TYPE]
    OPENEDEAR,
    CLOSEDEAR,
    EARTHRESHOLD,
    EYECLOSEDRATIO,
    FRAME
  };
}

extern unsigned char key[33];

#endif // PROTOCOLS_H