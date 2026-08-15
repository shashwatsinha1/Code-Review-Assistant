#pragma once

#include <string>

struct SourceLocationInfo {
    std::string file;
    int line = 0;
    int column = 0;

    bool isValid() const {
        return line > 0 && column > 0;
    }
};