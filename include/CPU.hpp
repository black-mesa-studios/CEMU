#pragma once
#include <string>
#include <cstdint>
#include <array>

#include "../include/Binary.hpp"
#include "../include/Bit.hpp"

namespace cpu {
    class CPU {
        public:
        std::array<std::uint16_t, 16> registers;
        std::array<std::uint8_t, 65536> memory;
        std::array<Instruction, 65536> instructionData;

        std::uint16_t pc;
        common::BitArray8 flags;

        int executeInstruction(std::uint8_t opcode, std::uint16_t arg1, std::uint16_t arg2);
        int loadProgram(const std::string& path);
        void step();
        void run();
    };
}