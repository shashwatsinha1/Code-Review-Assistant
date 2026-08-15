#pragma once

#include<string>

enum class Severity{
    INFO,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

enum class Category{
    BUG,
    PERFORMANCE,
    MEMORY,
    SECURITY,
    STYLE,
    LOGIC,
    BEST_PRACTICE
};

struct ReviewFinding{
    std::string ruleId;

    Severity severity;

    Category category;

    std::string file;

    int line;
    std::string code;
    std::string title;
    std::string description;
    std::string suggestion;
};