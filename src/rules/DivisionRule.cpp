#include "DivisionRule.h"

#include <regex>

std::string DivisionRule::getId() const {
    return "D001";
}

std::string DivisionRule::getName() const {
    return "Possible Division by Zero";
}

std::vector<ReviewFinding> DivisionRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    (void)context;

    std::vector<ReviewFinding> findings;

    const auto& lines =
        source.getLines();

    std::regex divisionPattern(
        R"(\b(\w+)\s*/\s*(\w+|0)\b)"
    );

    for (size_t i = 0; i < lines.size(); i++) {

        const std::string& line =
            lines[i];

        std::smatch match;

        // ------------------------------------
        // Check division
        // ------------------------------------

        if (std::regex_search(
                line,
                match,
                divisionPattern
            )) {

            std::string divisor =
                match[2].str();

            bool definitelyZero = false;

            // Direct division by zero.
            if (divisor == "0") {

                definitelyZero = true;
            }

            else {

                if (
                    context.getValueStateAt(
                        divisor,
                        static_cast<int>(i + 1)
                    ) == ValueState::ZERO
                ) {

                    definitelyZero = true;
                }
            }

            if (!definitelyZero) {
                continue;
            }

            ReviewFinding finding;

            finding.ruleId = getId();

            finding.severity =
                Severity::HIGH;

            finding.category =
                Category::BUG;

            finding.line =
                static_cast<int>(i + 1);

            finding.code =
                match[0].str();

            finding.title =
                "Possible division by zero";

            finding.description =
                "The divisor "
                + divisor
                + " is known to be zero at this point. "
                "The division may result in undefined "
                "behavior or a runtime error.";

            finding.suggestion =
                "Ensure the divisor is non-zero before "
                "performing the division.";

            findings.push_back(finding);
        }
    }

    return findings;
}
