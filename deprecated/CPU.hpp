#pragma once
#include <string>
#include <cstdint>

enum InstructionOpcode {
    NOP = 0x00,
    HLT = 0x01,
    MOV = 0x02,
    ADD = 0x03,
    SUB = 0x04,
    SHR = 0x05,
    SHL = 0x06,
    XOR = 0x07,
    OR  = 0x08,
    NOT = 0x09,
    AND = 0x0A,
    JMP = 0x0B,
    JZ  = 0x0C,
    CMP = 0x0D,
    MW  = 0x0E,
    MR  = 0x0F,
    LDI = 0x10,
    DDW = 0x11
};

class CPU {
    public:
    int executeInstruction(uint8_t opcode, uint16_t arg1, uint16_t arg2);
    int loadProgram(const std::string& path);
};