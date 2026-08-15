#include "NullPointerRule.h"
#include "test_helpers.h"

int main() {
    NullPointerRule rule;

    const std::string nullDereferenceCode =
        "int main() {\n"
        "    int* ptr = nullptr;\n"
        "    *ptr = 10;\n"
        "    return 0;\n"
        "}\n";

    auto nullFindings = runRule(rule, nullDereferenceCode);
    requireSize(nullFindings, 1, "N001 detects null pointer dereference");
    requireTrue(nullFindings[0].ruleId == "N001", "N001 rule id");
    requireTrue(nullFindings[0].severity == Severity::HIGH, "N001 severity");
    requireTrue(nullFindings[0].category == Category::BUG, "N001 category");

    const std::string arrowDereferenceCode =
        "struct Node { int value; };\n"
        "int main() {\n"
        "    Node* ptr = nullptr;\n"
        "    ptr->value = 10;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, arrowDereferenceCode),
        1,
        "N001 detects null arrow dereference"
    );

    const std::string validPointerCode =
        "#include <iostream>\n"
        "int main() {\n"
        "    int x = 10;\n"
        "    int* ptr = &x;\n"
        "    std::cout << *ptr;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, validPointerCode),
        0,
        "N001 ignores known valid pointer"
    );

    const std::string unknownPointerCode =
        "int* getPointer();\n"
        "int main() {\n"
        "    int* ptr = getPointer();\n"
        "    *ptr = 10;\n"
        "    return 0;\n"
        "}\n";

    requireSize(
        runRule(rule, unknownPointerCode),
        0,
        "N001 does not guess for unknown pointer state"
    );

    return 0;
}
