#include "ClangASTParser.h"

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
