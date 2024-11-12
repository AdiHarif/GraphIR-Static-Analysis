
#include <string>
#include <iostream>

extern "C" {

int32_t is_numeric(const char* str) {
    try {
        std::stof(str);
        return 1;
    }
    catch (...) {
        return 0;
    }
}

int32_t is_integer(const char* str) {
    try {
        std::stoi(str);
        return 1;
    }
    catch (...) {
        return 0;
    }

}

}
