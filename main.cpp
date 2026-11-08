#include <iostream>
#include <stdexcept>

#include "include/CPU.hpp"

int main(int argc, char **argv) {
    if(argc < 2) {
        throw std::runtime_error("Invalid launch options. [argc too short]\n");
        return 1;
    }
    printf("\033[2J\033[H");
    cpu::CPU *cpu = new cpu::CPU();
    int code = cpu->loadProgram(argv[1]);
    if(code != 0) {
        delete cpu;
        std::cout << "Failed loading program!\n";
        return 1;
    }
    cpu->run();
    delete cpu;
    std::cout << "\n\nProgram exited with code " << code << std::endl;

    return 0;
}