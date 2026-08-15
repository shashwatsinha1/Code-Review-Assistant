#include "SourceContext.h"
#include "test_helpers.h"

int main() {
    {
        SourceContext context(
            "int x = 10;\n"
        );

        const auto& symbols = context.getSymbols();
        auto it = symbols.find("x");

        requireTrue(it != symbols.end(), "symbol table contains x");
        requireTrue(it->second.name == "x", "x symbol name");
        requireTrue(it->second.type == "int", "x symbol type");
        requireTrue(it->second.declarationLine == 1, "x declaration line");
        requireTrue(it->second.initialized, "x initialized");
        requireTrue(it->second.value == "10", "x value");
        requireTrue(it->second.valueState == ValueState::NON_ZERO, "x non-zero");
        requireTrue(it->second.scopeDepth == 0, "x scope depth");
    }

    {
        SourceContext context(
            "int x;\n"
        );

        const auto& symbols = context.getSymbols();
        auto it = symbols.find("x");

        requireTrue(it != symbols.end(), "uninitialized x is present");
        requireTrue(!it->second.initialized, "x is uninitialized");
        requireTrue(it->second.valueState == ValueState::UNKNOWN, "x value unknown");
    }

    {
        SourceContext context(
            "int x = 10;\n"
            "x = 0;\n"
            "int y = x;\n"
        );

        requireTrue(
            context.getValueStateAt("x", 3) == ValueState::ZERO,
            "assignment updates x to zero"
        );

        requireTrue(
            context.getValueStateAt("y", 4) == ValueState::ZERO,
            "copy declaration propagates zero state"
        );
    }

    {
        SourceContext context(
            "int x = 0;\n"
            "x = 5;\n"
            "int y = x;\n"
        );

        requireTrue(
            context.getValueStateAt("x", 3) == ValueState::NON_ZERO,
            "reassignment updates x to non-zero"
        );

        requireTrue(
            context.getValueStateAt("y", 4) == ValueState::NON_ZERO,
            "copy declaration propagates non-zero state"
        );
    }

    {
        SourceContext context(
            "int x = 10;\n"
            "{\n"
            "    int x = 20;\n"
            "}\n"
        );

        int xCount = 0;
        bool foundOuter = false;
        bool foundInner = false;

        for (const auto& symbol : context.getAllSymbols()) {
            if (symbol.name != "x") {
                continue;
            }

            xCount++;

            if (
                symbol.value == "10" &&
                symbol.scopeDepth == 0
            ) {
                foundOuter = true;
            }

            if (
                symbol.value == "20" &&
                symbol.scopeDepth == 1
            ) {
                foundInner = true;
            }
        }

        requireTrue(xCount == 2, "scope tracking keeps both x symbols");
        requireTrue(foundOuter, "outer x is in scope depth 0");
        requireTrue(foundInner, "inner x is in scope depth 1");
        requireTrue(context.getScopes().size() >= 2, "scope table records nested scope");
    }

    {
        SourceContext context(
            "#include <vector>\n"
            "int main() {\n"
            "    std::vector<int> arr;\n"
            "    arr.push_back(10);\n"
            "    int x = arr[0];\n"
            "    return x;\n"
            "}\n"
        );

        requireTrue(
            context.isContainerKnownNonEmptyAt("arr", 5),
            "container state tracks push_back as non-empty"
        );
    }

    {
        SourceContext context(
            "int factorial(int n) {\n"
            "    if (n <= 1) {\n"
            "        return 1;\n"
            "    }\n"
            "    return n * factorial(n - 1);\n"
            "}\n"
        );

        const auto& functions =
            context.getFunctions();

        requireTrue(functions.size() == 1, "function parser records factorial");
        requireTrue(functions[0].name == "factorial", "function name is factorial");
        requireTrue(functions[0].line == 1, "function line is recorded");
        requireTrue(functions[0].recursive, "function parser detects recursion");
    }

    return 0;
}
