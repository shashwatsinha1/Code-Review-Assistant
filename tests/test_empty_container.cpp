#include "EmptyContainerRule.h"
#include "test_helpers.h"

int main() {
    EmptyContainerRule rule;

    const std::string firstElementCode =
        "#include <vector>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr;\n"
        "    int x = arr[0];\n"
        "    return x;\n"
        "}\n";

    auto firstElementFindings = runRule(rule, firstElementCode);
    requireSize(firstElementFindings, 1, "E001 detects arr[0]");
    requireTrue(firstElementFindings[0].ruleId == "E001", "E001 rule id");

    const std::string populatedFirstCode =
        "#include <vector>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr;\n"
        "    arr.push_back(10);\n"
        "    int x = arr[0];\n"
        "    return x;\n"
        "}\n";

    requireSize(
        runRule(rule, populatedFirstCode),
        0,
        "E001 treats push_back before arr[0] as known non-empty"
    );

    const std::string normalIndexCode =
        "#include <vector>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr;\n"
        "    int i = 0;\n"
        "    int x = arr[i];\n"
        "    return x;\n"
        "}\n";

    requireSize(
        runRule(rule, normalIndexCode),
        0,
        "E001 only reports first-element [0] access"
    );

    const std::string guardedCode =
        "#include <vector>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr;\n"
        "    if (!arr.empty()) {\n"
        "        int x = arr[0];\n"
        "        return x;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, guardedCode),
        0,
        "E001 ignores guarded arr[0]"
    );

    return 0;
}
