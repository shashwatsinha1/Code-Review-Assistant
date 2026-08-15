#include "DivisionRule.h"
#include "test_helpers.h"

int main() {
    DivisionRule rule;

    const std::string directZeroCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    int divisor = 0;\n"
        "    int result = x / divisor;\n"
        "    return result;\n"
        "}\n";

    auto directZeroFindings = runRule(rule, directZeroCode);
    requireSize(directZeroFindings, 1, "D001 detects known zero divisor");
    requireTrue(directZeroFindings[0].ruleId == "D001", "D001 rule id");
    requireTrue(directZeroFindings[0].severity == Severity::HIGH, "D001 severity");

    const std::string nonZeroCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    int divisor = 5;\n"
        "    int result = x / divisor;\n"
        "    return result;\n"
        "}\n";

    requireSize(
        runRule(rule, nonZeroCode),
        0,
        "D001 ignores known non-zero divisor"
    );

    const std::string zeroReassignmentCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    int divisor = 5;\n"
        "    divisor = 0;\n"
        "    int result = x / divisor;\n"
        "    return result;\n"
        "}\n";

    requireSize(
        runRule(rule, zeroReassignmentCode),
        1,
        "D001 detects reassignment to zero"
    );

    const std::string nonZeroReassignmentCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    int divisor = 0;\n"
        "    divisor = 5;\n"
        "    int result = x / divisor;\n"
        "    return result;\n"
        "}\n";

    requireSize(
        runRule(rule, nonZeroReassignmentCode),
        0,
        "D001 respects reassignment to non-zero"
    );

    const std::string copiedZeroCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    int divisor = 0;\n"
        "    int copied = divisor;\n"
        "    int result = x / copied;\n"
        "    return result;\n"
        "}\n";

    requireSize(
        runRule(rule, copiedZeroCode),
        1,
        "D001 uses SourceContext value tracking for copied zero values"
    );

    return 0;
}
