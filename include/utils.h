#ifndef UTILS_H
#define UTILS_H

int readNBytes(int fd, void* buf, int len);
int writeNBytes(int fd, const void* buf, int len);

#endif // UTILS_H