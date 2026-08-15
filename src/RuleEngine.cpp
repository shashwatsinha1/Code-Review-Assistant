#include "RuleEngine.h"

#include "SourceContext.h"

#include <algorithm>
#include <utility>

void RuleEngine::addRule(
    std::unique_ptr<Rule> rule
) {
    rules.push_back(
        std::move(rule)
    );
}

std::vector<ReviewFinding> RuleEngine::analyze(
    const SourceFile& source
) {
    std::vector<ReviewFinding> findings;

    // Create one context for the current source file.
    SourceContext context(
        source.getCode()
    );

    // Run every rule using the same context.
    for (auto& rule : rules) {

        std::vector<ReviewFinding> results =
            rule->check(
                source,
                context
            );

        findings.insert(
            findings.end(),
            results.begin(),
            results.end()
        );
    }

    // Sort findings by line number.
    std::sort(
        findings.begin(),
        findings.end(),
        [](const ReviewFinding& a,
           const ReviewFinding& b) {

            return a.line < b.line;
        }
    );

    return findings;
}