#include "../include/Terminal.hpp"
#include <termios.h>
#include <cstdio>
#include <unistd.h>

namespace term {
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
}