#pragma once
#include <cstdint>
#include <string>

namespace common {
    class BitArray8 {
        std::uint8_t _data;
        public:
        int getbit(int index);
        bool togglebit(int index);
        bool offbit(int index);
        bool onbit(int index);
        void setval(std::uint8_t value);
        std::uint8_t getval();
        std::string getbinrep();

        BitArray8(){ _data = 0; }
    };
}
