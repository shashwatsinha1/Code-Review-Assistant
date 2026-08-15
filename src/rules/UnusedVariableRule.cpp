#include "UnusedVariableRule.h"

#include <regex>
#include <set>

namespace {

bool isLocalVariableType(const std::string& type) {
    return (
        type == "int" ||
        type == "double" ||
        type == "float" ||
        type == "char" ||
        type == "bool" ||
        type == "string" ||
        type == "long" ||
        type == "vector" ||
        type.find("array") == 0
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

std::string UnusedVariableRule::getId() const {
    return "R001";
}

std::string UnusedVariableRule::getName() const {
    return "Possible Unused Variable";
}

std::vector<ReviewFinding> UnusedVariableRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    std::vector<ReviewFinding> findings;

    const auto& lines =
        source.getLines();

    std::set<std::string> reported;

    for (const auto& symbol : context.getAllSymbols()) {

        if (!isLocalVariableType(symbol.type)) {
            continue;
        }

        if (reported.count(symbol.name) > 0) {
            continue;
        }

        bool used = false;

        for (
            int lineNumber = symbol.declarationLine + 1;
            lineNumber <= source.lineCount();
            lineNumber++
        ) {

            if (
                containsUse(
                    lines[lineNumber - 1],
                    symbol.name
                )
            ) {

                used = true;
                break;
            }
        }

        if (used) {
            continue;
        }

        ReviewFinding finding;
        finding.ruleId = getId();
        finding.severity = Severity::LOW;
        finding.category = Category::STYLE;
        finding.line = symbol.declarationLine;
        finding.code =
            source.getLine(symbol.declarationLine);
        finding.title =
            "Possible unused variable";
        finding.description =
            "Variable '" +
            symbol.name +
            "' is declared but not used later.";
        finding.suggestion =
            "Remove '" +
            symbol.name +
            "' if it is unnecessary.";

        findings.push_back(finding);
        reported.insert(symbol.name);
    }

    return findings;
}
