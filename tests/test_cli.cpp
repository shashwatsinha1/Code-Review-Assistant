#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

struct CommandResult {
    int exitCode;
    std::string output;
};

std::string quote(const std::string& value) {
    return "\"" + value + "\"";
}

CommandResult runCommand(
    const std::string& arguments,
    const fs::path& outputFile
) {
    std::string command;

#ifdef _WIN32

    command =
        "cmd /C \"\"" +
        std::string(TEST_EXECUTABLE_PATH) +
        "\" " +
        arguments +
        " > " +
        quote(outputFile.string()) +
        " 2>&1\"";

#else

    command =
        quote(std::string(TEST_EXECUTABLE_PATH)) +
        " " +
        arguments +
        " > " +
        quote(outputFile.string()) +
        " 2>&1";

#endif

    int rawExitCode = std::system(command.c_str());

    int exitCode = rawExitCode;

#ifndef _WIN32

    if (rawExitCode == -1) {
        exitCode = -1;
    } else if (WIFEXITED(rawExitCode)) {
        exitCode = WEXITSTATUS(rawExitCode);
    } else if (WIFSIGNALED(rawExitCode)) {
        exitCode = 128 + WTERMSIG(rawExitCode);
    }

#endif

    std::ifstream output(outputFile);
    std::stringstream buffer;
    buffer << output.rdbuf();

    return {exitCode, buffer.str()};
}

void requireTrue(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAILED: " << message << "\n";
        std::exit(1);
    }
}

void requireExitCode(
    const CommandResult& result,
    int expected,
    const std::string& message
) {
    if (result.exitCode != expected) {
        std::cerr
            << "FAILED: "
            << message
            << " expected exit "
            << expected
            << ", got "
            << result.exitCode
            << "\nOutput:\n"
            << result.output
            << "\n";

        std::exit(1);
    }
}

void requireContains(
    const std::string& text,
    const std::string& expected,
    const std::string& message
) {
    requireTrue(
        text.find(expected) != std::string::npos,
        message + " missing: " + expected
    );
}

void requireAbsent(
    const std::string& text,
    const std::string& value,
    const std::string& message
) {
    requireTrue(
        text.find(value) == std::string::npos,
        message + " unexpectedly found: " + value
    );
}

int main() {
    const fs::path root(TEST_PROJECT_ROOT);
    const fs::path tempDir =
        fs::temp_directory_path() /
        "code_review_assistant_cli_tests";

    fs::create_directories(tempDir);

    int commandIndex = 0;

    auto run = [&](const std::string& arguments) {
        commandIndex++;
        return runCommand(
            arguments,
            tempDir / ("command_" + std::to_string(commandIndex) + ".txt")
        );
    };

    auto pathArg = [](const fs::path& path) {
        return quote(path.string());
    };

    const std::string sampleProject =
        pathArg(root / "test_projects" / "sample_project");

    const std::string cleanProject =
        pathArg(root / "test_projects" / "clean_project");

    auto normalScan = run(sampleProject);
    requireExitCode(normalScan, 1, "normal scan exits 1 when findings exist");

    auto cleanScan = run(cleanProject);
    requireExitCode(cleanScan, 0, "clean project exits 0");

    auto severityFilter = run("--severity HIGH " + sampleProject);
    requireExitCode(severityFilter, 1, "severity filter exits 1 for B001");
    requireContains(severityFilter.output, "B001", "severity filter keeps B001");
    requireAbsent(severityFilter.output, "P001", "severity filter removes P001");

    const fs::path defaultJson = root / "review_report.json";
    fs::remove(defaultJson);

    auto jsonScan = run("--json " + sampleProject);
    requireExitCode(jsonScan, 1, "json scan exits 1 when findings exist");
    requireTrue(fs::exists(defaultJson), "--json creates review_report.json");

    {
        std::ifstream file(defaultJson);
        json report = json::parse(file);
        requireTrue(report["totalIssues"] == 3, "default JSON totalIssues");
        requireTrue(report["findings"].is_array(), "default JSON findings array");
    }

    fs::remove(defaultJson);

    const fs::path customReport = tempDir / "custom_report.json";
    fs::remove(customReport);

    auto customOutput =
        run("--output " + pathArg(customReport) + " " + sampleProject);

    requireExitCode(
        customOutput,
        1,
        "custom output exits 1 when findings exist"
    );
    requireTrue(fs::exists(customReport), "--output creates custom JSON");

    {
        std::ifstream file(customReport);
        json report = json::parse(file);
        requireTrue(report["totalIssues"] == 3, "custom JSON totalIssues");
    }

    const fs::path htmlReport = tempDir / "review_report.html";
    fs::remove(htmlReport);

    auto htmlOutput =
        run("--html " + pathArg(htmlReport) + " " + sampleProject);

    requireExitCode(
        htmlOutput,
        1,
        "html output exits 1 when findings exist"
    );
    requireTrue(fs::exists(htmlReport), "--html creates HTML report");

    {
        std::ifstream file(htmlReport);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string html = buffer.str();

        requireContains(html, "<!doctype html>", "HTML report doctype");
        requireContains(html, "Code Review Report", "HTML report title");
        requireContains(html, "Total issues: 3", "HTML report total");
        requireContains(html, "B001", "HTML report contains rule ID");
    }

    auto invalidSeverity = run("--severity ABC " + sampleProject);
    requireExitCode(invalidSeverity, 2, "invalid severity exits 2");
    requireContains(invalidSeverity.output, "Error:", "invalid severity message");

    auto missingSeverity = run("--severity " + sampleProject);
    requireExitCode(missingSeverity, 2, "missing severity exits 2");

    auto missingOutput = run("--output " + sampleProject);
    requireExitCode(missingOutput, 2, "missing output filename exits 2");

    auto missingHtml = run("--html " + sampleProject);
    requireExitCode(missingHtml, 2, "missing html filename exits 2");

    auto unknownOption = run("--something " + sampleProject);
    requireExitCode(unknownOption, 2, "unknown option exits 2");

    auto multiplePaths = run(sampleProject + " " + cleanProject);
    requireExitCode(multiplePaths, 2, "multiple paths exit 2");

    auto noInput = run("");
    requireExitCode(noInput, 2, "no input exits 2");

    return 0;
}
