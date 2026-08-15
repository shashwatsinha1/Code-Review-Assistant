#include "ComplexityRule.h"
#include "test_helpers.h"

int main() {
    ComplexityRule rule;

    const std::string nestedLoopCode =
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    int n = 10;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        for (int j = 0; j < n; j++) {\n"
        "            std::cout << i + j;\n"
        "        }\n"
        "    }\n"
        "}\n";

    auto nestedFindings = runRule(rule, nestedLoopCode);
    requireSize(nestedFindings, 1, "P001 detects nested loops");
    requireTrue(nestedFindings[0].ruleId == "P001", "P001 rule id");
    requireTrue(nestedFindings[0].severity == Severity::MEDIUM, "P001 severity");
    requireTrue(
        nestedFindings[0].title.find("O(n^2)") != std::string::npos,
        "P001 includes O(n^2) estimate"
    );

    const std::string singleLoopCode =
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    int n = 10;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        std::cout << i;\n"
        "    }\n"
        "}\n";

    requireSize(
        runRule(rule, singleLoopCode),
        0,
        "P001 ignores single loops"
    );

    const std::string tripleLoopCode =
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    int n = 10;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        for (int j = 0; j < n; j++) {\n"
        "            for (int k = 0; k < n; k++) {\n"
        "                std::cout << i + j + k;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n";

    requireSize(
        runRule(rule, tripleLoopCode),
        2,
        "P001 reports each loop nested at depth two or deeper"
    );

    const std::string recursiveCode =
        "int factorial(int n) {\n"
        "    if (n <= 1) {\n"
        "        return 1;\n"
        "    }\n"
        "    return n * factorial(n - 1);\n"
        "}\n";

    auto recursiveFindings = runRule(rule, recursiveCode);
    requireSize(recursiveFindings, 1, "P002 detects recursion");
    requireTrue(recursiveFindings[0].ruleId == "P002", "P002 rule id");
    requireTrue(
        recursiveFindings[0].description.find("base case") != std::string::npos,
        "P002 explains base-case risk"
    );

    return 0;
}
