#include <cstdint>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <array>
#include "bit.hpp"
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

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

enum Flag {
    zerof = 0,
    haltf = 1
};

struct BinaryFormat {
    uint16_t magic;
    uint16_t entryPoint;
};

struct Instruction {
    uint8_t opcode;
    uint8_t __pad;
    uint16_t arg1;
    uint16_t arg2;
};

class CPU {
    public:
    std::array<uint16_t, 16> registers;
    std::array<uint8_t, 65536> memory;
    std::array<Instruction, 65536> instructionData;



    uint16_t pc;
    common::BitArray8 flags;

    int loadProgram(const std::string& path)
    {
        std::ifstream program(path, std::ios::binary);

        if (!program)
            throw std::runtime_error("Failed to open program");

        BinaryFormat format{};

        program.read(
            reinterpret_cast<char*>(&format.magic),
            sizeof(format.magic)
        );

        program.read(
            reinterpret_cast<char*>(&format.entryPoint),
            sizeof(format.entryPoint)
        );

        if (format.magic != 0x6734)
            return 2;

        program.seekg(0, std::ios::end);
        std::streamsize fileSize = program.tellg();

        if (fileSize < static_cast<std::streamsize>(sizeof(BinaryFormat)))
            throw std::runtime_error("Program is too small");

        size_t instructionSize =
            fileSize - sizeof(BinaryFormat);

        if (instructionSize > instructionData.size())
            throw std::runtime_error("Program doesnt fit in memory");

        program.seekg(sizeof(BinaryFormat), std::ios::beg);

        program.read(
            reinterpret_cast<char*>(instructionData.data()),
            instructionSize
        );

        if (!program)
            throw std::runtime_error("Failed to read program");


        pc = format.entryPoint;

        return 0;
    }

    int executeInstruction(uint8_t opcode, uint16_t arg1, uint16_t arg2);

    void step()
    {
        Instruction tmp = instructionData[pc];
        executeInstruction(tmp.opcode, tmp.arg1, tmp.arg2);
    }

    void run() {
        flags.offbit(haltf);
        while(!flags.getbit(haltf)) {
            step();
        }
    }

};

int CPU::executeInstruction(uint8_t opcode, uint16_t arg1, uint16_t arg2) {
    #define checkregisterbounds if(arg1 > 15 || arg2 > 15) {std::runtime_error("Register index out of bounds"); return 1;}
    bool increasePC = true;
    switch (opcode) {
        case NOP:
        break;

        case HLT:
        {
            flags.onbit(haltf);
            break;
        }

        case MOV:
        {
            checkregisterbounds
            registers[arg1] = registers[arg2];
            break;
        }

        case ADD:
        {
            checkregisterbounds
            registers[arg1] += registers[arg2];
            break;
        }

        case SUB:
        {
            checkregisterbounds
            registers[arg1] -= registers[arg2];
            break;
        }

        case SHR:
        {
            checkregisterbounds
            registers[arg1] >>= 1;
            break;
        }

        case SHL:
        {
            checkregisterbounds
            registers[arg1] <<= 1;
            break;
        }

        case XOR:
        {
            checkregisterbounds
            registers[arg1] ^= registers[arg2];
            break;
        }

        case OR:
        {
            checkregisterbounds
            registers[arg1] |= registers[arg2];
            break;
        }

        case NOT:
        {
            checkregisterbounds
            registers[arg1] = ~(registers[arg1]);
            break;
        }

        case AND:
        {
            checkregisterbounds
            registers[arg1] &= registers[arg2];
            break;
        }

        case JMP:
        {
            increasePC = false;
            pc = arg1;
            break;
        }

        case JZ:
        {
            if(flags.getbit(zerof) == 1) {
                increasePC = false;
                flags.offbit(zerof);
                pc = arg1;
            }
            break;
        }

        case CMP:
        {
            checkregisterbounds
            if(registers[arg1] == registers[arg2]) {
                flags.onbit(zerof);
            }
            break;
        }

        case MW:
        {
            if(arg2 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
            memory[arg1] = registers[arg2];
            break;
        }

        case MR:
        {
            if(arg1 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
            registers[arg1] = memory[arg2];
            break;
        }

        case LDI:
        {
            if(arg1 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
            registers[arg1] = arg2;
            break;
        }

        case DDW:
        {
            if(arg1 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
            putc(static_cast<char>(registers[arg1]), stdout);
            break;
        }
    }
    pc++;
    return 0;
}