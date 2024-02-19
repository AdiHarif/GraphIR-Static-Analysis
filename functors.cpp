
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

}
