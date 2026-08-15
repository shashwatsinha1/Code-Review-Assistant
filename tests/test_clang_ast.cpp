#include "ClangASTParser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void requireTrue(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAILED: " << message << "\n";
        std::exit(1);
    }
}

int main() {
    const fs::path tempFile =
        fs::temp_directory_path() /
        "code_review_assistant_ast_test.cpp";

    {
        std::ofstream file(tempFile);

        file << R"cpp(
int square(int x) {
    return x * x;
}

int main() {
    return square(5);
}
)cpp";
    }

    ClangASTParser parser;

    std::string diagnostics;

    const bool result =
        parser.parseFile(tempFile, diagnostics);

    requireTrue(
        result,
        "ClangASTParser should accept a valid C++ file"
    );

    requireTrue(
        diagnostics.empty(),
        "Valid source should not produce diagnostics"
    );

    fs::remove(tempFile);

    const fs::path invalidFile =
        fs::temp_directory_path() /
        "code_review_assistant_invalid.cpp";

    {
        std::ofstream file(invalidFile);

        file << R"cpp(
int brokenFunction() {
    return ;
    this is invalid C++;
}
)cpp";
    }

    diagnostics.clear();

    const bool invalidResult =
        parser.parseFile(invalidFile, diagnostics);

    requireTrue(
        !invalidResult,
        "ClangASTParser should reject invalid C++"
    );

    requireTrue(
        !diagnostics.empty(),
        "Invalid source should produce diagnostics"
    );

    fs::remove(invalidFile);

    return 0;
}
