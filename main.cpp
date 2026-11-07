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
#include <termios.h>
#include <unistd.h>

char getch()
{
    termios oldt{};

    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        perror("tcgetattr");
        return '\0';
    }

    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) {
        perror("tcsetattr");
        return '\0';
    }

    char c = 0;
    ssize_t result = read(STDIN_FILENO, &c, 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (result == -1)
        perror("read");

    return result == 1 ? c : '\0';
}

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
        std::cout << "Loading program from: " << path << std::endl;
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

        std::cout << "Program magic correct!\n";

        program.seekg(0, std::ios::end);
        std::streamsize fileSize = program.tellg();
        std::cout << "Got file size: " << fileSize << "\n";

        if (fileSize < static_cast<std::streamsize>(sizeof(BinaryFormat)))
            throw std::runtime_error("Program is too small");

        size_t instructionSize =
            fileSize - sizeof(BinaryFormat);
        std::cout << "Got instruction size: " << instructionSize << "\n";

        if (instructionSize > instructionData.size() * sizeof(Instruction))
            throw std::runtime_error("Program doesnt fit in memory");

        program.seekg(sizeof(BinaryFormat), std::ios::beg);

        std::cout << "Reading program into memory...\n";

        program.read(
            reinterpret_cast<char*>(instructionData.data()),
            instructionSize
        );

        std::cout << "Finished reading program into memory.\n";

        if (!program)
            throw std::runtime_error("Failed to read program");


        pc = format.entryPoint;
        std::cout << "Set PC to entry point: " << pc << std::endl;

        return 0;
    }

    int executeInstruction(uint8_t opcode, uint16_t arg1, uint16_t arg2);

    void step()
    {
        Instruction tmp = instructionData[pc];
        // std::cout << "Executing instruction at PC: " << pc << std::endl;
        // std::cout << "Opcode: " << static_cast<int>(tmp.opcode) << std::endl;
        // std::cout << "Arg1: " << tmp.arg1 << std::endl;
        // std::cout << "Arg2: " << tmp.arg2 << std::endl;
        executeInstruction(tmp.opcode, tmp.arg1, tmp.arg2);
    }

    void run() {
        std::cout << "Press enter to start execution..." << std::endl;
        std::cin.ignore();
        printf("\033[2J\033[H");
        std::cout << "Starting execution!\n";
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

        case STP:
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

        case MSG:
        {

            if (arg2 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }

            if (arg1 == 0x0000) {
                putc(static_cast<char>(registers[arg2]), stdout);
                fflush(stdout);
            }
            else if (arg1 == 0x0001) {
                char c = getch();
                registers[arg2] = static_cast<uint16_t>(static_cast<unsigned char>(c));
            }
            else if (arg1 == 0x0002) {
                char c = getch();

                registers[arg2] = static_cast<unsigned char>(c);

                putc(c, stdout);
                fflush(stdout);
            }
            else {
                throw std::runtime_error("Invalid MSG argument");
            }

            break;
        }

        case MLDI:
        {
            memory[arg1] = static_cast<uint8_t>(arg2);
            break;
        }

        case MUL:
        {
            checkregisterbounds
            registers[arg1] *= registers[arg2];
            break;
        }

        case DIV:
        {
            checkregisterbounds
            registers[arg1] /= registers[arg2];
            break;
        }

        case MOD:
        {
            checkregisterbounds
            registers[arg1] %= registers[arg2];
            break;
        }

        case INC:
        {
            if(arg1 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
                
            registers[arg1]++;
            break;
        }

        case DEC:
        {
            if(arg1 > 15) {
                std::runtime_error("Register index out of bounds"); return 1;
            }
                
            registers[arg1]--;
            break;
        }

    }
    if(increasePC)
        pc++;
    return 0;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        throw std::runtime_error("Invalid launch options. [argc too short]\n");
        return 1;
    }
    printf("\033[2J\033[H");
    CPU *cpu = new CPU();
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