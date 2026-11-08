#pragma once
#include <cstdint>

enum InstructionOpcode {
    NOP = 0x00,
    STP = 0x01,
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
    MSG = 0x11,
    MLDI =0x12,
    MUL = 0x13,
    DIV = 0x14,
    MOD = 0x15,
    INC = 0x16,
    DEC = 0x17
};

enum Flag {
    zerof = 0,
    haltf = 1
};

struct BinaryFormat {
    std::uint16_t magic;
    std::uint16_t entryPoint;
};

struct Instruction {
    std::uint8_t opcode;
    std::uint8_t __pad;
    std::uint16_t arg1;
    std::uint16_t arg2;
};