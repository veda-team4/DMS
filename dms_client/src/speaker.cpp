#include "speaker.h"
#include <cstdlib>
#include <string>
#include <iostream>

Speaker::Speaker(const std::string& dev) : device(dev) {
    std::cout << "[INIT] Speaker construction! device = " << device << std::endl;
}

Speaker::~Speaker() {
    std::cout << "[INIT] Speaker desturction! device = " << device << std::endl;
}

void Speaker::play(const std::string& filename)
{
    std::string cmd = "aplay -D" + device + " \"" + "./../../dms_client/resources/sounds/" + filename + "\"";
    std::cout << "[play] start " << filename << "!!" << std::endl;

    int ret = system(cmd.c_str());
    if(ret == 0){
        std::cout << "[play] 재생 완료" << std::endl;
    }
    else{
        std::cout << "[play] 재생 실패" << std::endl;
    }
} 