#include <string>
#include "../include/Bit.hpp"

namespace common {
    int BitArray8::getbit(int index) {
        if (index < 0 || index >= 8)
            return -1;
        return (_data & (1u << index)) >> index;
    }

    bool BitArray8::togglebit(int index) {
        if (index < 0 || index >= 8)
            return false;
        _data ^= (1u << index);
        return true;
    }

    bool BitArray8::offbit(int index) {
        if (index < 0 || index >= 8)
            return false;
        _data &= ~(1u << index);
        return true;
    }

    bool BitArray8::onbit(int index) {
        if (index < 0 || index >= 8)
            return false;
        _data |= (1u << index);
        return true;
    }

    void BitArray8::setval(std::uint8_t value) {
        _data = value;
    }

    std::uint8_t BitArray8::getval() {
        return _data;
    }

    std::string BitArray8::getbinrep() {
        std::string out = "";
        for(int i = 7; i >= 0; i--) {
            out += std::to_string(static_cast<int>(
                getbit(i)
            ));
        }
        return out;
    }
}
