#pragma once

#include <filesystem>
#include <string>

class ClangASTParser {
public:
    ClangASTParser() = default;

    bool parseFile(
        const std::filesystem::path& sourceFile,
        std::string& diagnostics
    ) const;
};
