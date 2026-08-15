#include "MemoryLeakRule.h"
#include "test_helpers.h"

int main() {
    MemoryLeakRule rule;

    const std::string leakCode =
        "int main() {\n"
        "    int* p = new int;\n"
        "    return 0;\n"
        "}\n";

    auto leakFindings = runRule(rule, leakCode);
    requireSize(leakFindings, 1, "M001 detects missing delete");
    requireTrue(leakFindings[0].ruleId == "M001", "M001 rule id");
    requireTrue(leakFindings[0].severity == Severity::MEDIUM, "M001 severity");
    requireTrue(leakFindings[0].category == Category::MEMORY, "M001 category");

    const std::string deletedCode =
        "int main() {\n"
        "    int* p = new int;\n"
        "    delete p;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, deletedCode),
        0,
        "M001 ignores allocation with matching delete"
    );

    const std::string arrayDeletedCode =
        "int main() {\n"
        "    int* p = new int[10];\n"
        "    delete[] p;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, arrayDeletedCode),
        0,
        "M001 recognizes delete[] for new[]"
    );

    const std::string arrayLeakCode =
        "int main() {\n"
        "    int* p = new int[10];\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, arrayLeakCode),
        1,
        "M001 detects missing delete[] for new[] allocation"
    );

    return 0;
}
