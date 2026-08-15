#include "ComplexityRule.h"

#include <regex>

std::string ComplexityRule::getId() const {
    return "P001";
}

std::string ComplexityRule::getName() const {
    return "Possible O(n^2) Complexity";
}

std::vector<ReviewFinding> ComplexityRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    (void)source;

    std::vector<ReviewFinding> findings;

    const auto& loops =
        context.getLoops();

    for (const auto& loop : loops) {

        // A loop with depth >= 2 is nested.
        if (loop.depth < 2) {
            continue;
        }

        ReviewFinding finding;

        finding.ruleId = getId();

        finding.severity = Severity::MEDIUM;

        finding.category =
            Category::PERFORMANCE;

        finding.line = loop.line;

        finding.code =
            "for (...; "
            + loop.condition
            + "; ...)";

        finding.title =
            "Possible O(n^2) complexity";

        finding.description =
            "Nested loops were detected. "
            "If both loops iterate over the same input size, "
            "this may result in O(n^2) time complexity.";

        finding.suggestion =
            "Consider whether the nested iteration "
            "can be optimized.";

        findings.push_back(finding);
    }

    return findings;
}