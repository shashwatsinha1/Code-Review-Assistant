#include "UninitializedRule.h"

#include <regex>
#include <set>

namespace {

bool isPrimitiveType(const std::string& type) {
    return (
        type == "int" ||
        type == "double" ||
        type == "float" ||
        type == "char" ||
        type == "bool" ||
        type == "string" ||
        type == "long"
    );
}

bool containsUse(
    const std::string& line,
    const std::string& name
) {
    std::regex usePattern(
        "\\b" + name + "\\b"
    );

    return std::regex_search(line, usePattern);
}

}

std::string UninitializedRule::getId() const {
    return "U001";
}

std::string UninitializedRule::getName() const {
    return "Possible Uninitialized Variable Use";
}

std::vector<ReviewFinding> UninitializedRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    std::vector<ReviewFinding> findings;

    const auto& lines =
        source.getLines();

    std::set<std::string> reported;

    for (const auto& symbol : context.getAllSymbols()) {

        if (
            !isPrimitiveType(symbol.type) ||
            symbol.initialized
        ) {
            continue;
        }

        bool initialized = false;

        std::regex assignmentPattern(
            "\\b" + symbol.name + R"(\s*=\s*([^;]+)\s*;)"
        );

        for (
            int lineNumber = symbol.declarationLine + 1;
            lineNumber <= source.lineCount();
            lineNumber++
        ) {

            const std::string& line =
                lines[lineNumber - 1];

            std::smatch match;

            if (
                std::regex_search(
                    line,
                    match,
                    assignmentPattern
                )
            ) {

                initialized = true;
                continue;
            }

            if (
                !initialized &&
                containsUse(line, symbol.name)
            ) {

                if (reported.count(symbol.name) > 0) {
                    break;
                }

                ReviewFinding finding;
                finding.ruleId = getId();
                finding.severity = Severity::HIGH;
                finding.category = Category::BUG;
                finding.line = lineNumber;
                finding.code = line;
                finding.title =
                    "Possible uninitialized variable use";
                finding.description =
                    "Variable '" +
                    symbol.name +
                    "' may be used before it is initialized.";
                finding.suggestion =
                    "Initialize '" +
                    symbol.name +
                    "' before using it.";

                findings.push_back(finding);
                reported.insert(symbol.name);
                break;
            }
        }
    }

    return findings;
}
