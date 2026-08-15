#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include "ReviewService.h"
#include "ReviewReport.h"
#include "ReviewFinding.h"

void printHelp() {

    std::cout
        << "========================================\n"
        << "       CODE REVIEW ASSISTANT\n"
        << "========================================\n\n"

        << "Usage:\n"
        << "  CodeReviewAssistant <path> [options]\n\n"

        << "Options:\n"
        << "  --json\n"
        << "      Save the review report as "
        << "review_report.json\n\n"
        << "  --output FILE\n"
        << "      Save the JSON report to the specified file.\n"
        << "      Automatically enables JSON output.\n\n"

        << "  --html FILE\n"
        << "      Save the review report as an HTML file.\n\n"

        << "  --severity LEVEL\n"
        << "      Show issues at or above the specified severity.\n\n"

        << "      Levels:\n"
        << "        INFO\n"
        << "        LOW\n"
        << "        MEDIUM\n"
        << "        HIGH\n"
        << "        CRITICAL\n\n"

        << "  --help\n"
        << "      Show this help message.\n\n"

        << "Exit codes:\n"
        << "  0  Clean review with no findings.\n"
        << "  1  Review completed and findings were detected.\n"
        << "  2  CLI usage error or analyzer/tool failure.\n\n"

        << "Examples:\n"
        << "  CodeReviewAssistant test_projects/sample_project\n\n"

        << "  CodeReviewAssistant "
        << "test_projects/sample_project --severity HIGH\n\n"

        << "  CodeReviewAssistant "
        << "test_projects/sample_project --json\n\n"

        << "  CodeReviewAssistant "
        << "test_projects/sample_project --output my_report.json\n\n"

        << "  CodeReviewAssistant "
        << "test_projects/sample_project --severity HIGH --json\n\n"

        << "  CodeReviewAssistant "
        << "test_projects/sample_project --html report.html\n";
}

int main(int argc, char* argv[]) {

    // ----------------------------------------
    // Help
    // ----------------------------------------

    if (
        argc >= 2 &&
        std::string(argv[1]) == "--help"
    ) {

        printHelp();

        return 0;
    }

    // ----------------------------------------
    // Command-line arguments
    // ----------------------------------------

    std::string inputPath;

    bool saveJson = false;

    bool saveHtml = false;

    std::string severityFilter;

    std::string outputFile =
        "review_report.json";

    std::string htmlFile;

    for (int i = 1; i < argc; i++) {

        std::string argument = argv[i];

        // --json
        if (argument == "--json") {

            saveJson = true;
        }

        // --output FILE
        else if (argument == "--output") {

            if (
                i + 1 >= argc ||
                std::string(argv[i + 1]).rfind("--", 0) == 0
            ) {

                std::cerr
                    << "Error: --output requires a filename.\n";

                return 2;
            }

            outputFile = argv[++i];

            saveJson = true;
        }

        // --html FILE
        else if (argument == "--html") {

            if (
                i + 1 >= argc ||
                std::string(argv[i + 1]).rfind("--", 0) == 0
            ) {

                std::cerr
                    << "Error: --html requires a filename.\n";

                return 2;
            }

            htmlFile = argv[++i];

            saveHtml = true;
        }

        // --severity LEVEL
        else if (argument == "--severity") {

            if (
                i + 1 >= argc ||
                std::string(argv[i + 1]).rfind("--", 0) == 0
            ) {

                std::cerr
                    << "Error: --severity requires a level.\n";

                return 2;
            }

            severityFilter = argv[++i];
        }

        // --help
        else if (argument == "--help") {

            printHelp();

            return 0;
        }

        // Unknown option
        else if (
            argument.size() >= 2 &&
            argument[0] == '-' &&
            argument[1] == '-'
        ) {

            std::cerr
                << "Error: Unknown option: "
                << argument
                << "\n";

            return 2;
        }

        // Input path
        else {

            if (!inputPath.empty()) {

                std::cerr
                    << "Error: Multiple input paths provided.\n";

                return 2;
            }

            inputPath = argument;
        }
    }

    // ----------------------------------------
    // No input path
    // ----------------------------------------

    if (inputPath.empty()) {

        std::cerr
            << "Usage: CodeReviewAssistant "
            << "<file-or-directory> "
            << "[--json] "
            << "[--output FILE] "
            << "[--html FILE] "
            << "[--severity LEVEL]\n";

        return 2;
    }

    Severity selectedSeverity;

    bool hasSeverityFilter = false;

    if (!severityFilter.empty()) {

        hasSeverityFilter = true;

        if (severityFilter == "INFO") {

            selectedSeverity = Severity::INFO;
        }

        else if (severityFilter == "LOW") {

            selectedSeverity = Severity::LOW;
        }

        else if (severityFilter == "MEDIUM") {

            selectedSeverity = Severity::MEDIUM;
        }

        else if (severityFilter == "HIGH") {

            selectedSeverity = Severity::HIGH;
        }

        else if (severityFilter == "CRITICAL") {

            selectedSeverity = Severity::CRITICAL;
        }

        else {

            std::cerr
                << "Error: Invalid severity '"
                << severityFilter
                << "'.\n";

            return 2;
        }
    }

    // ----------------------------------------
    // Run review
    // ----------------------------------------

    ReviewService service;

    std::vector<ReviewFinding> findings;

    try {

        findings =
            service.review(inputPath);
    }

    catch (const std::exception& error) {

        std::cerr
            << "Error: "
            << error.what()
            << "\n";

        return 2;
    }

    // ----------------------------------------
    // Severity filtering
    // ----------------------------------------

    if (hasSeverityFilter) {

        std::vector<ReviewFinding> filteredFindings;

        for (const auto& finding : findings) {

            if (
                finding.severity >=
                selectedSeverity
            ) {

                filteredFindings.push_back(
                    finding
                );
            }
        }

        findings = filteredFindings;
    }

    // ----------------------------------------
    // Generate report
    // ----------------------------------------

    ReviewReport report(findings);

    report.print();

    // ----------------------------------------
    // JSON output
    // ----------------------------------------

    if (saveJson) {

        if (!report.saveJson(outputFile)) {

            return 2;
        }

        std::cout
            << "\nJSON report saved to "
            << outputFile
            << "\n";
    }

    // ----------------------------------------
    // HTML output
    // ----------------------------------------

    if (saveHtml) {

        if (!report.saveHtml(htmlFile)) {

            return 2;
        }

        std::cout
            << "\nHTML report saved to "
            << htmlFile
            << "\n";
    }

    if (!findings.empty()) {
        return 1;
    }
    return 0;
}
