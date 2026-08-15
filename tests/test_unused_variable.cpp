#include "UnusedVariableRule.h"
#include "test_helpers.h"

int main() {
    UnusedVariableRule rule;

    const std::string unusedCode =
        "int main() {\n"
        "    int x = 10;\n"
        "    return 0;\n"
        "}\n";

    auto unusedFindings = runRule(rule, unusedCode);
    requireSize(unusedFindings, 1, "R001 detects unused local variable");
    requireTrue(unusedFindings[0].ruleId == "R001", "R001 rule id");
    requireTrue(unusedFindings[0].severity == Severity::LOW, "R001 severity");
    requireTrue(unusedFindings[0].category == Category::STYLE, "R001 category");

    const std::string usedCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int x = 10;\n"
        "    std::cout << x;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, usedCode),
        0,
        "R001 ignores used local variable"
    );

    const std::string parameterCode =
        "int main(int argc, char* argv[]) {\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, parameterCode),
        0,
        "R001 does not report function parameters"
    );

    const std::string multipleCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int used = 1;\n"
        "    int unused = 2;\n"
        "    std::cout << used;\n"
        "    return 0;\n"
        "}\n";

    auto multipleFindings = runRule(rule, multipleCode);
    requireSize(multipleFindings, 1, "R001 reports only the unused variable");
    requireTrue(
        multipleFindings[0].description.find("unused") != std::string::npos,
        "R001 names the unused variable"
    );

    return 0;
}
