#include "BoundsRule.h"
#include "test_helpers.h"

int main() {
    BoundsRule rule;

    const std::string badCode =
        "#include <vector>\n"
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr(10);\n"
        "    for (int i = 0; i <= arr.size(); i++) {\n"
        "        std::cout << arr[i];\n"
        "    }\n"
        "}\n";

    auto badFindings = runRule(rule, badCode);
    requireSize(badFindings, 1, "B001 detects i <= arr.size()");
    requireTrue(badFindings[0].ruleId == "B001", "B001 rule id");
    requireTrue(badFindings[0].severity == Severity::HIGH, "B001 severity");
    requireTrue(badFindings[0].category == Category::BUG, "B001 category");
    requireTrue(badFindings[0].line == 6, "B001 line number");

    const std::string correctCode =
        "#include <vector>\n"
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> arr(10);\n"
        "    for (int i = 0; i < arr.size(); i++) {\n"
        "        std::cout << arr[i];\n"
        "    }\n"
        "}\n";

    requireSize(
        runRule(rule, correctCode),
        0,
        "B001 ignores i < arr.size()"
    );

    const std::string numericBoundaryCode =
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    int arr[10];\n"
        "    for (int i = 0; i <= 10; i++) {\n"
        "        std::cout << arr[i];\n"
        "    }\n"
        "}\n";

    requireSize(
        runRule(rule, numericBoundaryCode),
        0,
        "B001 currently does not claim numeric-bound detection"
    );

    const std::string unrelatedCode =
        "int main() {\n"
        "    int x = 1;\n"
        "    int y = 2;\n"
        "    if (x <= y) {\n"
        "        return x;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, unrelatedCode),
        0,
        "B001 ignores unrelated <= expressions"
    );

    return 0;
}
