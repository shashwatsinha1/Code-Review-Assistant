#include "EmptyContainerRule.h"
#include <algorithm>
#include <regex>

std::string EmptyContainerRule::getId() const {
    return "E001";
}

std::string EmptyContainerRule::getName() const {
    return "Possible Empty-Container Access";
}

std::vector<ReviewFinding> EmptyContainerRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    std::vector<ReviewFinding> findings;

    const auto& variables =
        context.getVariables();

    const auto& accesses =
        context.getArrayAccesses();

    const auto& lines =
        source.getLines();

    for (const auto& access : accesses) {

        // We only care about [0]
        if (access.index != "0") {
            continue;
        }

        // Check whether the container is known
        // to be a vector or array.
        bool knownContainer = false;

        for (const auto& variable : variables) {

            if (variable.name == access.container) {

                if (
                    variable.type == "vector" ||
                    variable.type.find("array") == 0
                ) {

                    knownContainer = true;
                }

                break;
            }
        }

        if (!knownContainer) {
            continue;
        }

        if (
            context.isContainerKnownNonEmptyAt(
                access.container,
                access.line
            )
        ) {

            continue;
        }

        // ------------------------------------
        // Check for an empty() guard
        // ------------------------------------

        bool guarded = false;

        int accessLine = access.line;

        // Look at the previous few lines.
        int startLine =
            std::max(0, accessLine - 4);

        for (int i = startLine;
             i < accessLine;
             i++) {

            const std::string& previousLine =
                lines[i];

            std::string emptyGuard =
                "!"
                + access.container
                + ".empty()";

            std::string sizeGuard =
                access.container
                + ".size() > 0";

            std::string sizeGuardReverse =
                "0 < "
                + access.container
                + ".size()";

            if (
                previousLine.find(emptyGuard)
                    != std::string::npos
                ||
                previousLine.find(sizeGuard)
                    != std::string::npos
                ||
                previousLine.find(sizeGuardReverse)
                    != std::string::npos
            ) {

                guarded = true;
                break;
            }
        }

        // Don't report a known-safe access.
        if (guarded) {
            continue;
        }

        // ------------------------------------
        // Create finding
        // ------------------------------------

        ReviewFinding finding;

        finding.ruleId = getId();

        finding.severity = Severity::MEDIUM;

        finding.category = Category::BUG;

        finding.line = access.line;

        finding.code =
            access.container
            + "["
            + access.index
            + "]";

        finding.title =
            "Possible empty-container access";

        finding.description =
            "Detected expression: "
            + finding.code
            + "\n"
            "The container may be empty when "
            "its first element is accessed.";

        finding.suggestion =
            "Check !"
            + access.container
            + ".empty() before accessing "
            "the first element.";

        findings.push_back(finding);
    }

    return findings;
}
