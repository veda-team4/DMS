#ifndef SPEAKER_H
#define SPEAKER_H

#include <string>

class Speaker {
public:
  Speaker(const std::string& dev = "plughw:3,0");
  ~Speaker();

private:
  std::string device;

public:
    void play(const std::string& filename);
};

#endif