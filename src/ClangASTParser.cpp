#include "ClangASTParser.h"
#include <regex>
#include <vector>

#include "VariableInfo.h"
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace {

std::string quoteArgument(const std::string& value) {
    return "\"" + value + "\"";
}

FILE* openPipe(const std::string& command) {
#ifdef _WIN32
    return _popen(command.c_str(), "r");
#else
    return popen(command.c_str(), "r");
#endif
}

int closePipe(FILE* pipe) {
#ifdef _WIN32
    return _pclose(pipe);
#else
    return pclose(pipe);
#endif
}

} // namespace

bool ClangASTParser::parseFile(
    const std::filesystem::path& sourceFile,
    std::string& diagnostics
) const {
    diagnostics.clear();

    if (!std::filesystem::exists(sourceFile)) {
        diagnostics =
            "Source file does not exist: " +
            sourceFile.string();

        return false;
    }

    const std::string command =
        "clang++ -fsyntax-only " +
        quoteArgument(sourceFile.string()) +
        " 2>&1";

    FILE* pipe = openPipe(command);

    if (!pipe) {
        diagnostics = "Failed to start clang++.";
        return false;
    }

    std::ostringstream output;
    char buffer[256];

    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        output << buffer;
    }

    const int rawExitCode = closePipe(pipe);

    diagnostics = output.str();

#ifdef _WIN32
    const int exitCode = rawExitCode;
#else
    const int exitCode = WEXITSTATUS(rawExitCode);
#endif

    return exitCode == 0;
}
bool ClangASTParser::extractVariables(
    const std::filesystem::path& sourceFile,
    std::vector<VariableInfo>& variables,
    std::string& diagnostics
) const {
    variables.clear();
    diagnostics.clear();

    if (!std::filesystem::exists(sourceFile)) {
        diagnostics =
            "Source file does not exist: " +
            sourceFile.string();

        return false;
    }

    const std::string command =
        "clang++ -Xclang -ast-dump -fsyntax-only " +
        quoteArgument(sourceFile.string()) +
        " 2>&1";

    FILE* pipe = openPipe(command);

    if (!pipe) {
        diagnostics = "Failed to start clang++.";
        return false;
    }

    std::ostringstream output;
    char buffer[1024];

    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        output << buffer;
    }

    const int rawExitCode = closePipe(pipe);

#ifdef _WIN32
    const int exitCode = rawExitCode;
#else
    const int exitCode =
        WIFEXITED(rawExitCode)
            ? WEXITSTATUS(rawExitCode)
            : -1;
#endif

    const std::string astOutput = output.str();

    if (exitCode != 0) {
        diagnostics = astOutput;
        return false;
    }

    std::istringstream stream(astOutput);
    std::string line;

    while (std::getline(stream, line)) {
        const bool isParameter =
            line.find("ParmVarDecl") != std::string::npos;

        const bool isField =
            line.find("FieldDecl") != std::string::npos;

        const bool isVariable =
            line.find("VarDecl") != std::string::npos;

        if (!isParameter && !isField && !isVariable) {
            continue;
        }

        VariableInfo variable;
        std::smatch match;

        static const std::regex locationRegex(
            R"((?:<|line:)(?:[^:>]*:)?(\d+):(\d+))"
        );

        if (std::regex_search(line, match, locationRegex)) {
            variable.location.file = sourceFile.string();
            variable.location.line = std::stoi(match[1].str());
            variable.location.column = std::stoi(match[2].str());
        }

        static const std::regex declarationRegex(
            R"((?:VarDecl|ParmVarDecl|FieldDecl)[^'\n]*\s([A-Za-z_][A-Za-z0-9_]*)\s+'([^']+)')"
        );

        if (!std::regex_search(line, match, declarationRegex)) {
            continue;
        }

        variable.name = match[1].str();
        variable.type = match[2].str();

        variable.isParameter = isParameter;
        variable.isField = isField;

        variable.isLocal =
            !variable.isParameter &&
            !variable.isField;

        variable.hasInitializer =
            line.find(" cinit") != std::string::npos ||
            line.find(" listinit") != std::string::npos;

        variable.isUsed =
            line.find(" used ") != std::string::npos;

        variables.push_back(variable);
    }

    return true;
}