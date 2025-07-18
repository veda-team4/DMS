#include "speaker.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <thread>

Speaker::Speaker(const std::string& dev) : device(dev) {
}

Speaker::~Speaker() {
}

void Speaker::play(const std::string& filename)
{
    std::thread([this, filename](){
        std::string cmd = "aplay -D" + device + " \"" + "./../../dms_client/resources/sounds/" + filename + "\"";

        int ret = system(cmd.c_str());
        if (ret == 0) {}
        else {}
    }).detach();

} 