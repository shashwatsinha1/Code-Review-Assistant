#include "ReviewReport.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <map>
using json = nlohmann::json;
std::string severityToString(Severity severity) {

    switch (severity) {

        case Severity::INFO:
            return "INFO";

        case Severity::LOW:
            return "LOW";

        case Severity::MEDIUM:
            return "MEDIUM";

        case Severity::HIGH:
            return "HIGH";

        case Severity::CRITICAL:
            return "CRITICAL";
    }

    return "UNKNOWN";
}

std::string categoryToString(Category category) {

    switch (category) {

        case Category::BUG:
            return "BUG";

        case Category::PERFORMANCE:
            return "PERFORMANCE";

        case Category::MEMORY:
            return "MEMORY";

        case Category::SECURITY:
            return "SECURITY";

        case Category::STYLE:
            return "STYLE";

        case Category::LOGIC:
            return "LOGIC";

        case Category::BEST_PRACTICE:
            return "BEST_PRACTICE";
    }

    return "UNKNOWN";
}
json findingToJson(const ReviewFinding& finding) {

    json j;
    j["file"] = finding.file;
    j["code"] = finding.code;
    j["ruleId"] = finding.ruleId;

    j["severity"] =
        severityToString(finding.severity);

    j["category"] =
        categoryToString(finding.category);

    j["line"] = finding.line;

    j["title"] = finding.title;

    j["description"] =
        finding.description;

    j["suggestion"] =
        finding.suggestion;

    return j;
}
ReviewReport::ReviewReport(
    const std::vector<ReviewFinding>& findings
)
    : findings(findings) {
}

int ReviewReport::totalIssues() const {
    return static_cast<int>(findings.size());
}
void ReviewReport::printSeveritySummary() const {

    std::map<Severity, int> counts;

    for (const auto& finding : findings) {

        counts[finding.severity]++;
    }

    std::cout << "\nSeverity Summary:\n";

    std::cout
        << "  INFO:     "
        << counts[Severity::INFO]
        << "\n";

    std::cout
        << "  LOW:      "
        << counts[Severity::LOW]
        << "\n";

    std::cout
        << "  MEDIUM:   "
        << counts[Severity::MEDIUM]
        << "\n";

    std::cout
        << "  HIGH:     "
        << counts[Severity::HIGH]
        << "\n";

    std::cout
        << "  CRITICAL: "
        << counts[Severity::CRITICAL]
        << "\n";
}
void ReviewReport::printRuleSummary() const {

    std::map<std::string, int> counts;

    for (const auto& finding : findings) {

        counts[finding.ruleId]++;
    }

    std::cout << "\nRule Summary:\n";

    for (const auto& entry : counts) {

        std::cout
            << "  "
            << entry.first
            << ": "
            << entry.second
            << "\n";
    }
}
void ReviewReport::print() const {

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "          CODE REVIEW REPORT\n";
    std::cout << "========================================\n";

    std::cout << "\nIssues Found: "
              << findings.size()
              << "\n\n";

    for (const auto& finding : findings) {

        std::cout << "["
                  << severityToString(finding.severity)
                  << "] "
                  << finding.file
                  << ":"
                  << finding.line
                  << "\n";

        std::cout << "Rule: "
                  << finding.ruleId
                  << "\n";

        std::cout << "Code: "
                  << finding.code
                  << "\n";
        
        std::cout << "Category: "
                  << categoryToString(finding.category)
                  << "\n";

        std::cout << "Title: "
                  << finding.title
                  << "\n";

        std::cout << "Description: "
                  << finding.description
                  << "\n";

        std::cout << "Suggestion: "
                  << finding.suggestion
                  << "\n";

        std::cout << "----------------------------------------\n";
    }

    std::cout << "\nTotal issues: "
          << totalIssues()
          << "\n";

    printSeveritySummary();

    printRuleSummary();
}
bool ReviewReport::saveJson(
    const std::string& filename
) const {

    json report;

    report["totalIssues"] =
        findings.size();

    report["findings"] =
        json::array();

    for (const auto& finding : findings) {

        report["findings"].push_back(
            findingToJson(finding)
        );
    }

    std::ofstream file(filename);

    if (!file) {

        std::cerr
            << "Error: Could not open JSON file: "
            << filename
            << "\n";

        return false;
    }

    file << report.dump(4);

    file.close();

    return true;
}
