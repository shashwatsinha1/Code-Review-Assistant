#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "VariableInfo.h"

class ClangASTParser {
public:
    ClangASTParser() = default;

    bool parseFile(
        const std::filesystem::path& sourceFile,
        std::string& diagnostics
    ) const;

    bool extractVariables(
        const std::filesystem::path& sourceFile,
        std::vector<VariableInfo>& variables,
        std::string& diagnostics
    ) const;
};