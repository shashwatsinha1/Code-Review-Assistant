#pragma once

#include <string>

#include "SourceLocationInfo.h"

struct VariableInfo {
    std::string name;
    std::string type;
    SourceLocationInfo location;

    bool isLocal = false;
    bool isParameter = false;
    bool isField = false;
    bool hasInitializer = false;
    bool isUsed = false;
};