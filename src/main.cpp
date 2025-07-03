#include <iostream>
#include <cstring>
#include "threads.h"
#include "functions.h"

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cout << "Usage: %s <type>" << std::endl;
    return -1;
  }
  if (strcmp(argv[1], "camsetpage") == 0) {
    int ret = camsetpage();
    if (ret == -1) {
      std::cout << "camsetpage error" << std::endl;
      return -1;
    }
    return 0;
  }
  return 0;
}