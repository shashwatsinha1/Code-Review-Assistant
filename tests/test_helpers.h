#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ReviewFinding.h"
#include "Rule.h"
#include "SourceContext.h"
#include "SourceFile.h"

inline std::vector<ReviewFinding> runRule(
    Rule& rule,
    const std::string& code
) {
    SourceFile source(code);
    SourceContext context(code);

    return rule.check(source, context);
}

inline void requireTrue(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        std::cerr << "FAILED: " << message << "\n";
        std::exit(1);
    }
}

inline void requireSize(
    const std::vector<ReviewFinding>& findings,
    size_t expected,
    const std::string& message
) {
    if (findings.size() != expected) {
        std::cerr
            << "FAILED: "
            << message
            << " expected "
            << expected
            << " finding(s), got "
            << findings.size()
            << "\n";

        for (const auto& finding : findings) {
            std::cerr
                << "  "
                << finding.ruleId
                << " line "
                << finding.line
                << ": "
                << finding.code
                << "\n";
        }

        std::exit(1);
    }
}

inline bool hasRule(
    const std::vector<ReviewFinding>& findings,
    const std::string& ruleId
) {
    return std::any_of(
        findings.begin(),
        findings.end(),
        [&](const ReviewFinding& finding) {
            return finding.ruleId == ruleId;
        }
    );
}
