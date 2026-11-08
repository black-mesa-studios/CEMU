#include <stdexcept>
#include <array>
#include <iostream>
#include <fstream>

#include "../include/Bit.hpp"
#include "../include/Binary.hpp"
#include "../include/CPU.hpp"
#include "../include/Terminal.hpp"

namespace cpu {
    int CPU::loadProgram(const std::string& path) {
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

    void CPU::run() {
        std::cout << "Press enter to start execution..." << std::endl;
        std::cin.ignore();
        printf("\033[2J\033[H");
        std::cout << "Starting execution!\n";
        flags.offbit(haltf);
        while(!flags.getbit(haltf)) {
            step();
        }
    }

    void CPU::step()
    {
        Instruction tmp = instructionData[pc];
        // std::cout << "Executing instruction at PC: " << pc << std::endl;
        // std::cout << "Opcode: " << static_cast<int>(tmp.opcode) << std::endl;
        // std::cout << "Arg1: " << tmp.arg1 << std::endl;
        // std::cout << "Arg2: " << tmp.arg2 << std::endl;
        executeInstruction(tmp.opcode, tmp.arg1, tmp.arg2);
    }

    int CPU::executeInstruction(std::uint8_t opcode, std::uint16_t arg1, std::uint16_t arg2) {
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
                throw std::runtime_error("Register index out of bounds"); return 1;
                }
                memory[arg1] = registers[arg2];
                break;
            }

            case MR:
            {
                if(arg1 > 15) {
                throw std::runtime_error("Register index out of bounds"); return 1;
                }
                registers[arg1] = memory[arg2];
                break;
            }

            case LDI:
            {
                if(arg1 > 15) {
                throw std::runtime_error("Register index out of bounds"); return 1;
                }
                registers[arg1] = arg2;
                break;
            }

            case MSG:
            {

                if (arg2 > 15) {
                throw std::runtime_error("Register index out of bounds"); return 1;
                }

                if (arg1 == 0x0000) {
                    putc(static_cast<char>(registers[arg2]), stdout);
                    fflush(stdout);
                }
                else if (arg1 == 0x0001) {
                    char c = term::getch();
                    registers[arg2] = static_cast<std::uint16_t>(static_cast<unsigned char>(c));
                }
                else if (arg1 == 0x0002) {
                    char c = term::getch();

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
                memory[arg1] = static_cast<std::uint8_t>(arg2);
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
                throw std::runtime_error("Register index out of bounds"); return 1;
                }
                    
                registers[arg1]++;
                break;
            }

            case DEC:
            {
                if(arg1 > 15) {
                throw std::runtime_error("Register index out of bounds"); return 1;
                }
                    
                registers[arg1]--;
                break;
            }

        }
        if(increasePC)
            pc++;
        return 0;
    }
}