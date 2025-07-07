#ifndef PROTOCOLS_H
#define PROTOCOLS_H

/*
통신 간에 사용할 프로토콜 정의
프로토콜은 아래와 같다
[Type(1byte)][Len(4byte)][Data]

첫 1바이트는 프로토콜 타입을,
이후 4바이트는 Data의 byte 크기를
이후에는 Data가 전송된다

types.h 에는 첫 1바이트에 해당하는 Type 을 정의한다
*/

#define COMMAND 0
#define FRAME 1
#define OPENEDEAR 2
#define CLOSEDEAR 3
#define EARTHRESHOLD 4
#define EYECLOSEDRATIO 5

#endif // PROTOCOLS_H