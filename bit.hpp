#pragma once
#include <cstdint>
#include <string>

namespace common {
    class BitArray8 {
        uint8_t _data;
        public:
        int getbit(int index) {
            if (index < 0 || index >= 8)
                return -1;
            return (_data & (1u << index)) >> index;
        }

        bool togglebit(int index) {
            if (index < 0 || index >= 8)
                return false;
            _data ^= (1u << index);
            return true;
        }

        bool offbit(int index) {
            if (index < 0 || index >= 8)
                return false;
            _data &= ~(1u << index);
            return true;
        }

        bool onbit(int index) {
            if (index < 0 || index >= 8)
                return false;
            _data |= (1u << index);
            return true;
        }

        void setval(uint8_t value) {
            _data = value;
        }

        uint8_t getval() {
            return _data;
        }

        std::string getbinrep() {
            std::string out = "";
            for(int i = 7; i >= 0; i--) {
                out += std::to_string(static_cast<int>(
                    getbit(i)
                ));
            }
            return out;
        }

        BitArray8()
            { _data = 0; }
    };
}
