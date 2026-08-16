#include "ClangASTParser.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int main() {
    const auto tempFile =
        std::filesystem::temp_directory_path() /
        "code_review_variable_ast_test.cpp";

    {
        std::ofstream file(tempFile);

        file << R"(
int globalValue = 42;

class User {
public:
    int age = 20;
    int id;
};

int calculate(int input) {
    int result = input + 10;
    int unused;

    return result;
}

int main() {
    int count = 5;
    return count;
}
)";
    }

    ClangASTParser parser;

    std::vector<VariableInfo> variables;
    std::string diagnostics;

    const bool success =
        parser.extractVariables(
            tempFile,
            variables,
            diagnostics
        );

    assert(success);
    assert(diagnostics.empty());

    bool foundGlobal = false;
    bool foundAge = false;
    bool foundId = false;
    bool foundInput = false;
    bool foundResult = false;
    bool foundUnused = false;
    bool foundCount = false;

    for (const auto& variable : variables) {
        if (variable.name == "globalValue") {
            foundGlobal = true;

            assert(variable.type == "int");
            assert(variable.hasInitializer);
            assert(!variable.isParameter);
            assert(!variable.isField);
        }

        if (variable.name == "age") {
            foundAge = true;

            assert(variable.type == "int");
            assert(variable.isField);
            assert(!variable.isParameter);
            assert(variable.hasInitializer);
        }

        if (variable.name == "id") {
            foundId = true;

            assert(variable.type == "int");
            assert(variable.isField);
            assert(!variable.hasInitializer);
        }

        if (variable.name == "input") {
            foundInput = true;

            assert(variable.type == "int");
            assert(variable.isParameter);
            assert(variable.isUsed);
            assert(!variable.isField);
        }

        if (variable.name == "result") {
            foundResult = true;

            assert(variable.type == "int");
            assert(variable.hasInitializer);
            assert(variable.isUsed);
            assert(!variable.isParameter);
            assert(!variable.isField);
        }

        if (variable.name == "unused") {
            foundUnused = true;

            assert(variable.type == "int");
            assert(!variable.hasInitializer);
            assert(!variable.isUsed);
        }

        if (variable.name == "count") {
            foundCount = true;

            assert(variable.type == "int");
            assert(variable.hasInitializer);
            assert(variable.isUsed);
        }
    }

    assert(foundGlobal);
    assert(foundAge);
    assert(foundId);
    assert(foundInput);
    assert(foundResult);
    assert(foundUnused);
    assert(foundCount);

    std::filesystem::remove(tempFile);

    return 0;
}