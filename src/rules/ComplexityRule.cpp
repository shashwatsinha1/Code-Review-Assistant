#include "ComplexityRule.h"

#include <string>

namespace {

std::string complexityForDepth(int depth) {

    if (depth <= 1) {
        return "O(n)";
    }

    return "O(n^" + std::to_string(depth) + ")";
}

}

std::string ComplexityRule::getId() const {
    return "P001";
}

std::string ComplexityRule::getName() const {
    return "Complexity and Algorithm Analysis";
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

        std::string estimatedComplexity =
            complexityForDepth(loop.depth);

        finding.title =
            "Possible " +
            estimatedComplexity +
            " complexity";

        finding.description =
            "Nested loops were detected. "
            "If both loops iterate over the same input size, "
            "this may result in approximately "
            + estimatedComplexity
            + " time complexity.";

        finding.suggestion =
            "Consider whether the nested iteration "
            "can be optimized using hashing, sorting, "
            "two pointers, early exits, or precomputed lookup data.";

        findings.push_back(finding);
    }

    for (const auto& function : context.getFunctions()) {

        if (!function.recursive) {
            continue;
        }

        ReviewFinding finding;

        finding.ruleId = "P002";

        finding.severity = Severity::MEDIUM;

        finding.category =
            Category::PERFORMANCE;

        finding.line = function.line;

        finding.code =
            function.name + "(...)";

        finding.title =
            "Recursive function detected";

        finding.description =
            "Function '" +
            function.name +
            "' appears to call itself. Recursive algorithms can be "
            "correct, but they should have a clear base case and "
            "their time and stack-space complexity should be checked.";

        finding.suggestion =
            "Verify the base case, estimate the recursion depth, "
            "and consider an iterative approach if stack usage or "
            "repeated subproblems may become expensive.";

        findings.push_back(finding);
    }

    return findings;
}
