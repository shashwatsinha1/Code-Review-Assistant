#include "UninitializedRule.h"
#include "test_helpers.h"

int main() {
    UninitializedRule rule;

    const std::string badCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int x;\n"
        "    std::cout << x;\n"
        "    return 0;\n"
        "}\n";

    auto badFindings = runRule(rule, badCode);
    requireSize(badFindings, 1, "U001 detects uninitialized variable use");
    requireTrue(badFindings[0].ruleId == "U001", "U001 rule id");
    requireTrue(badFindings[0].severity == Severity::HIGH, "U001 severity");
    requireTrue(badFindings[0].category == Category::BUG, "U001 category");

    const std::string directInitializerCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int x = 10;\n"
        "    std::cout << x;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, directInitializerCode),
        0,
        "U001 ignores direct initializer"
    );

    const std::string assignmentBeforeUseCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int x;\n"
        "    x = 10;\n"
        "    std::cout << x;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, assignmentBeforeUseCode),
        0,
        "U001 ignores assignment before use"
    );

    const std::string copyInitializationCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int other = 10;\n"
        "    int x;\n"
        "    x = other;\n"
        "    std::cout << x;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, copyInitializationCode),
        0,
        "U001 treats assignment from initialized symbol as initialized"
    );

    const std::string multipleVariablesCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int good = 1;\n"
        "    int bad;\n"
        "    std::cout << good;\n"
        "    std::cout << bad;\n"
        "    return 0;\n"
        "}\n";

    auto multipleFindings = runRule(rule, multipleVariablesCode);
    requireSize(
        multipleFindings,
        1,
        "U001 identifies the uninitialized variable among several"
    );
    requireTrue(
        multipleFindings[0].description.find("bad") != std::string::npos,
        "U001 finding names the bad variable"
    );

    return 0;
}
