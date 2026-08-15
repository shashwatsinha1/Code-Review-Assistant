#include "NullPointerRule.h"

#include <regex>
#include <unordered_map>

std::string NullPointerRule::getId() const {
    return "N001";
}

std::string NullPointerRule::getName() const {
    return "Possible Null Pointer Dereference";
}

std::vector<ReviewFinding> NullPointerRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    (void)context;

    std::vector<ReviewFinding> findings;

    const auto& lines =
        source.getLines();

    std::unordered_map<std::string, int> pointerState;

    std::regex nullDeclaration(
        R"(\b(?:[\w:<>]+)\s*\*\s*(\w+)\s*=\s*(nullptr|NULL|0)\s*;)"
    );

    std::regex addressDeclaration(
        R"(\b(?:[\w:<>]+)\s*\*\s*(\w+)\s*=\s*&\s*\w+\s*;)"
    );

    std::regex pointerDeclaration(
        R"(\b(?:[\w:<>]+)\s*\*\s*(\w+)\s*(?:=\s*[^;]+)?;)"
    );

    std::regex nullAssignment(
        R"(\b(\w+)\s*=\s*(nullptr|NULL|0)\s*;)"
    );

    std::regex addressAssignment(
        R"(\b(\w+)\s*=\s*&\s*\w+\s*;)"
    );

    for (size_t i = 0; i < lines.size(); i++) {

        const std::string& line =
            lines[i];

        std::smatch match;

        if (
            std::regex_search(
                line,
                match,
                nullDeclaration
            )
        ) {

            pointerState[match[1].str()] = 0;
            continue;
        }

        if (
            std::regex_search(
                line,
                match,
                addressDeclaration
            )
        ) {

            pointerState[match[1].str()] = 1;
            continue;
        }

        if (
            std::regex_search(
                line,
                match,
                pointerDeclaration
            )
        ) {

            pointerState[match[1].str()] = 2;
            continue;
        }

        if (
            std::regex_search(
                line,
                match,
                nullAssignment
            ) &&
            pointerState.find(match[1].str()) != pointerState.end()
        ) {

            pointerState[match[1].str()] = 0;
            continue;
        }

        if (
            std::regex_search(
                line,
                match,
                addressAssignment
            ) &&
            pointerState.find(match[1].str()) != pointerState.end()
        ) {

            pointerState[match[1].str()] = 1;
            continue;
        }

        for (const auto& pointer : pointerState) {

            if (pointer.second != 0) {
                continue;
            }

            std::regex dereferencePattern(
                std::string(R"((\*\s*)") +
                pointer.first +
                R"(\b|\b)" +
                pointer.first +
                R"(\s*->))"
            );

            if (
                !std::regex_search(
                    line,
                    match,
                    dereferencePattern
                )
            ) {
                continue;
            }

            ReviewFinding finding;
            finding.ruleId = getId();
            finding.severity = Severity::HIGH;
            finding.category = Category::BUG;
            finding.line = static_cast<int>(i + 1);
            finding.code = line;
            finding.title =
                "Possible null pointer dereference";
            finding.description =
                "Pointer '" +
                pointer.first +
                "' is known to be null when dereferenced.";
            finding.suggestion =
                "Check that '" +
                pointer.first +
                "' is not null before dereferencing it.";

            findings.push_back(finding);
            break;
        }
    }

    return findings;
}
