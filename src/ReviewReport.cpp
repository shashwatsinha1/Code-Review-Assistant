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

std::string escapeHtml(const std::string& value) {

    std::string escaped;

    for (char ch : value) {

        switch (ch) {

            case '&':
                escaped += "&amp;";
                break;

            case '<':
                escaped += "&lt;";
                break;

            case '>':
                escaped += "&gt;";
                break;

            case '"':
                escaped += "&quot;";
                break;

            case '\'':
                escaped += "&#39;";
                break;

            default:
                escaped += ch;
                break;
        }
    }

    return escaped;
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

bool ReviewReport::saveHtml(
    const std::string& filename
) const {

    std::ofstream file(filename);

    if (!file) {

        std::cerr
            << "Error: Could not open HTML file: "
            << filename
            << "\n";

        return false;
    }

    file
        << "<!doctype html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "  <meta charset=\"utf-8\">\n"
        << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        << "  <title>Code Review Report</title>\n"
        << "  <style>\n"
        << "    body { font-family: Arial, sans-serif; margin: 0; background: #f5f7fb; color: #172033; }\n"
        << "    header { background: #172033; color: white; padding: 32px; }\n"
        << "    main { max-width: 1080px; margin: 0 auto; padding: 24px; }\n"
        << "    h1 { margin: 0 0 8px; font-size: 32px; }\n"
        << "    .summary { font-size: 18px; color: #dce6f7; }\n"
        << "    .finding { background: white; border: 1px solid #d9e0ea; border-radius: 8px; margin: 16px 0; padding: 18px; }\n"
        << "    .meta { display: flex; flex-wrap: wrap; gap: 8px; margin-bottom: 12px; }\n"
        << "    .badge { border-radius: 999px; padding: 4px 10px; font-size: 12px; font-weight: bold; }\n"
        << "    .severity { background: #ffe8cc; color: #8a3b00; }\n"
        << "    .category { background: #e8f0ff; color: #17458f; }\n"
        << "    .rule { background: #ecf8ef; color: #1b6934; }\n"
        << "    .location { color: #4d5b73; margin-bottom: 12px; }\n"
        << "    pre { background: #111827; color: #f8fafc; padding: 12px; border-radius: 6px; overflow-x: auto; }\n"
        << "    p { line-height: 1.5; }\n"
        << "    .empty { background: white; border: 1px solid #d9e0ea; border-radius: 8px; padding: 18px; }\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <header>\n"
        << "    <h1>Code Review Report</h1>\n"
        << "    <div class=\"summary\">Total issues: "
        << findings.size()
        << "</div>\n"
        << "  </header>\n"
        << "  <main>\n";

    if (findings.empty()) {

        file
            << "    <section class=\"empty\">\n"
            << "      <h2>No issues found</h2>\n"
            << "      <p>The analyzer did not report any findings.</p>\n"
            << "    </section>\n";
    }

    for (const auto& finding : findings) {

        file
            << "    <section class=\"finding\">\n"
            << "      <div class=\"meta\">\n"
            << "        <span class=\"badge severity\">"
            << escapeHtml(severityToString(finding.severity))
            << "</span>\n"
            << "        <span class=\"badge category\">"
            << escapeHtml(categoryToString(finding.category))
            << "</span>\n"
            << "        <span class=\"badge rule\">"
            << escapeHtml(finding.ruleId)
            << "</span>\n"
            << "      </div>\n"
            << "      <h2>"
            << escapeHtml(finding.title)
            << "</h2>\n"
            << "      <div class=\"location\">"
            << escapeHtml(finding.file)
            << ":"
            << finding.line
            << "</div>\n"
            << "      <pre><code>"
            << escapeHtml(finding.code)
            << "</code></pre>\n"
            << "      <h3>Description</h3>\n"
            << "      <p>"
            << escapeHtml(finding.description)
            << "</p>\n"
            << "      <h3>Suggestion</h3>\n"
            << "      <p>"
            << escapeHtml(finding.suggestion)
            << "</p>\n"
            << "    </section>\n";
    }

    file
        << "  </main>\n"
        << "</body>\n"
        << "</html>\n";

    file.close();

    return true;
}
