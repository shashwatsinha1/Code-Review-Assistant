#include <memory>

#include "BoundsRule.h"
#include "ComplexityRule.h"
#include "DivisionRule.h"
#include "EmptyContainerRule.h"
#include "MemoryLeakRule.h"
#include "NullPointerRule.h"
#include "RuleEngine.h"
#include "SourceFile.h"
#include "UninitializedRule.h"
#include "UnusedVariableRule.h"
#include "test_helpers.h"

int main() {
    const std::string cleanCode =
        "#include <vector>\n"
        "\n"
        "int main() {\n"
        "    std::vector<int> numbers;\n"
        "    numbers.push_back(10);\n"
        "\n"
        "    for (int i = 0; i < 1; i++) {\n"
        "        numbers.push_back(i);\n"
        "    }\n"
        "\n"
        "    return 0;\n"
        "}\n";

    RuleEngine engine;
    engine.addRule(std::make_unique<BoundsRule>());
    engine.addRule(std::make_unique<EmptyContainerRule>());
    engine.addRule(std::make_unique<ComplexityRule>());
    engine.addRule(std::make_unique<DivisionRule>());
    engine.addRule(std::make_unique<UninitializedRule>());
    engine.addRule(std::make_unique<UnusedVariableRule>());
    engine.addRule(std::make_unique<NullPointerRule>());
    engine.addRule(std::make_unique<MemoryLeakRule>());

    SourceFile source(cleanCode);
    requireSize(
        engine.analyze(source),
        0,
        "clean code has no findings"
    );

    return 0;
}
