#include "BoundsRule.h"

#include <regex>

std::string BoundsRule::getId() const {
    return "B001";
}

std::string BoundsRule::getName() const {
    return "Possible Out-of-Bounds Access";
}

std::vector<ReviewFinding> BoundsRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    (void)source;

    std::vector<ReviewFinding> findings;

    const auto& loops =
        context.getLoops();

    const auto& accesses =
        context.getArrayAccesses();

    for (const auto& loop : loops) {

        std::regex pattern(
            R"(\b(\w+)\s*<=\s*(\w+)\.size\s*\(\s*\))"
        );

        std::smatch match;

        if (!std::regex_search(
                loop.condition,
                match,
                pattern
            )) {

            continue;
        }

        std::string indexVariable =
            match[1].str();

        std::string container =
            match[2].str();

        // Make sure the condition uses
        // the same variable as the loop.
        if (indexVariable != loop.variable) {
            continue;
        }

        // Check whether the loop actually
        // accesses container[indexVariable].
        bool accessesContainer = false;

        for (const auto& access : accesses) {

            if (
                access.container == container &&
                access.index == indexVariable &&
                access.line > loop.line
            ) {

                accessesContainer = true;
                break;
            }
        }

        if (!accessesContainer) {
            continue;
        }

        ReviewFinding finding;

        finding.ruleId = getId();

        finding.severity =
            Severity::HIGH;

        finding.category =
            Category::BUG;

        finding.line =
            loop.line;

        finding.code =
            match[0].str();

        finding.title =
            "Possible out-of-bounds access";

        finding.description =
            "Detected expression: "
            + finding.code
            + "\n"
            "The loop may allow "
            + indexVariable
            + " to become equal to "
            + container
            + ".size(), while the last valid "
            "index is size() - 1.";

        finding.suggestion =
            "Consider changing:\n    "
            + finding.code
            + "\nto use < instead of <=.";

        findings.push_back(finding);
    }

    return findings;
}