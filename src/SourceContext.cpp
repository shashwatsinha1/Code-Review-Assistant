#include "SourceContext.h"

#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

ValueState stateForValue(const std::string& value) {
    if (value == "0") {
        return ValueState::ZERO;
    }

    if (std::regex_match(value, std::regex(R"(-?\d+)"))) {
        return ValueState::NON_ZERO;
    }

    return ValueState::UNKNOWN;
}

std::string stateValue(ValueState state) {
    if (state == ValueState::ZERO) {
        return "0";
    }

    return "";
}

}

SourceContext::SourceContext(
    const std::string& code
) {
    std::string line;

    int lineNumber = 0;

    int braceDepth = 0;

    std::vector<int> activeLoopDepths;

    scopes.emplace_back();

    std::vector<int> activeScopes;
    activeScopes.push_back(0);

    auto currentScopeIndex = [&]() {
        return activeScopes.back();
    };

    auto currentScopeDepth = [&]() {
        return static_cast<int>(activeScopes.size()) - 1;
    };

    auto recordSymbol =
        [&](SymbolInfo symbol) {
            symbols[symbol.name] = symbol;
            scopes[currentScopeIndex()][symbol.name] = symbol;
            allSymbols.push_back(symbol);
        };

    auto recordValue =
        [&](const std::string& name,
            int currentLine,
            ValueState state,
            const std::string& value) {
            valueHistory.push_back(
                {name, currentLine, state, value}
            );

            auto it = symbols.find(name);

            if (it != symbols.end()) {
                it->second.initialized = true;
                it->second.valueState = state;
                it->second.value = value;
            }
        };

    auto recordContainer =
        [&](const std::string& name,
            int currentLine,
            bool maybeEmpty) {
            containerHistory.push_back(
                {name, currentLine, maybeEmpty}
            );

            auto it = symbols.find(name);

            if (it != symbols.end()) {
                it->second.maybeEmpty = maybeEmpty;
            }
        };

    std::regex vectorDeclaration(
        R"(\b(?:const\s+)?(?:std::)?vector\s*<\s*[\w:]+\s*>\s*(?:&|\*)?\s*(\w+))"
    );

    std::regex arrayDeclaration(
        R"(\b(\w+)\s+(\w+)\s*\[\s*(\d+)\s*\])"
    );

    std::regex primitiveDeclaration(
        R"(\b(int|double|float|char|bool|string|long)\s+(\w+)\s*(?:=\s*([^;]+))?;)"
    );

    std::regex numericAssignment(
        R"(\b(\w+)\s*=\s*(-?\d+)\s*;)"
    );

    std::regex unknownAssignment(
        R"(\b(\w+)\s*=\s*([^;]+)\s*;)"
    );

    std::regex variableCopyDeclaration(
        R"(\b(?:int|double|float|long)\s+(\w+)\s*=\s*([A-Za-z_]\w*)\s*;)"
    );

    std::regex pushBackPattern(
        R"(\b(\w+)\s*\.\s*push_back\s*\()"
    );

    std::regex clearPattern(
        R"(\b(\w+)\s*\.\s*clear\s*\()"
    );

    std::regex loopPattern(
        R"(\bfor\s*\(\s*(?:int\s+)?(\w+)\s*=\s*[^;]+;\s*([^;]+);)"
    );

    std::regex accessPattern(
        R"(\b(\w+)\s*\[\s*([^\]]+)\s*\])"
    );

    std::istringstream parser(code);

    while (std::getline(parser, line)) {

        lineNumber++;

        std::smatch match;

        if (std::regex_search(
                line,
                match,
                vectorDeclaration
            )) {

            VariableInfo variable;
            variable.name = match[1].str();
            variable.type = "vector";
            variable.line = lineNumber;
            variables.push_back(variable);

            SymbolInfo symbol;
            symbol.name = variable.name;
            symbol.type = "vector";
            symbol.declarationLine = lineNumber;
            symbol.initialized = true;
            symbol.value = "";
            symbol.scopeDepth = currentScopeDepth();
            symbol.valueState = ValueState::UNKNOWN;
            symbol.maybeEmpty = true;

            recordSymbol(symbol);
            recordContainer(symbol.name, lineNumber, true);
        }

        if (std::regex_search(
                line,
                match,
                arrayDeclaration
            )) {

            VariableInfo variable;
            variable.name = match[2].str();
            variable.type =
                "array[" + match[3].str() + "]";
            variable.line = lineNumber;
            variables.push_back(variable);

            SymbolInfo symbol;
            symbol.name = variable.name;
            symbol.type = variable.type;
            symbol.declarationLine = lineNumber;
            symbol.initialized = true;
            symbol.value = "";
            symbol.scopeDepth = currentScopeDepth();
            symbol.valueState = ValueState::UNKNOWN;
            symbol.maybeEmpty = false;

            recordSymbol(symbol);
            recordContainer(symbol.name, lineNumber, false);
        }

        if (std::regex_search(
                line,
                match,
                primitiveDeclaration
            )) {

            VariableInfo variable;
            variable.name = match[2].str();
            variable.type = "primitive";
            variable.line = lineNumber;
            variables.push_back(variable);

            SymbolInfo symbol;
            symbol.name = variable.name;
            symbol.type = match[1].str();
            symbol.declarationLine = lineNumber;
            symbol.initialized = match[3].matched;
            symbol.value =
                match[3].matched ? match[3].str() : "";
            symbol.scopeDepth = currentScopeDepth();
            symbol.valueState =
                match[3].matched
                    ? stateForValue(match[3].str())
                    : ValueState::UNKNOWN;
            symbol.maybeEmpty = false;

            if (std::regex_search(
                    line,
                    match,
                    variableCopyDeclaration
                )) {

                ValueState copiedState =
                    getValueStateAt(match[2].str(), lineNumber);

                symbol.valueState = copiedState;
                symbol.value = stateValue(copiedState);
            }

            recordSymbol(symbol);

            if (symbol.initialized) {
                recordValue(
                    symbol.name,
                    lineNumber,
                    symbol.valueState,
                    symbol.value
                );
            }
        }

        if (std::regex_search(
                line,
                match,
                numericAssignment
            )) {

            recordValue(
                match[1].str(),
                lineNumber,
                stateForValue(match[2].str()),
                match[2].str()
            );
        }
        else if (
            std::regex_search(
                line,
                match,
                unknownAssignment
            ) &&
            !std::regex_search(line, primitiveDeclaration)
        ) {

            recordValue(
                match[1].str(),
                lineNumber,
                ValueState::UNKNOWN,
                ""
            );
        }

        if (std::regex_search(
                line,
                match,
                pushBackPattern
            )) {

            recordContainer(match[1].str(), lineNumber, false);
        }

        if (std::regex_search(
                line,
                match,
                clearPattern
            )) {

            recordContainer(match[1].str(), lineNumber, true);
        }

        if (std::regex_search(
                line,
                match,
                loopPattern
            )) {

            LoopInfo loop;
            loop.variable = match[1].str();
            loop.condition = match[2].str();
            loop.line = lineNumber;
            loop.depth =
                static_cast<int>(activeLoopDepths.size()) + 1;
            loops.push_back(loop);

            activeLoopDepths.push_back(
                braceDepth + 1
            );
        }

        if (std::regex_search(
                line,
                match,
                accessPattern
            )) {

            ArrayAccessInfo access;
            access.container = match[1].str();
            access.index = match[2].str();
            access.line = lineNumber;
            arrayAccesses.push_back(access);
        }

        for (char ch : line) {

            if (ch == '{') {

                braceDepth++;
                scopes.emplace_back();
                activeScopes.push_back(
                    static_cast<int>(scopes.size()) - 1
                );
            }

            else if (ch == '}') {

                braceDepth--;

                if (activeScopes.size() > 1) {
                    activeScopes.pop_back();
                }

                while (
                    !activeLoopDepths.empty() &&
                    braceDepth <
                        activeLoopDepths.back()
                ) {

                    activeLoopDepths.pop_back();
                }
            }
        }
    }
}

const std::vector<VariableInfo>&
SourceContext::getVariables() const {

    return variables;
}

const std::vector<LoopInfo>&
SourceContext::getLoops() const {

    return loops;
}

const std::vector<ArrayAccessInfo>&
SourceContext::getArrayAccesses() const {

    return arrayAccesses;
}

const std::unordered_map<std::string, SymbolInfo>&
SourceContext::getSymbols() const {

    return symbols;
}

const std::vector<
    std::unordered_map<std::string, SymbolInfo>
>& SourceContext::getScopes() const {

    return scopes;
}

const std::vector<SymbolInfo>&
SourceContext::getAllSymbols() const {

    return allSymbols;
}

ValueState SourceContext::getValueStateAt(
    const std::string& name,
    int line
) const {

    ValueState state = ValueState::UNKNOWN;

    for (const auto& change : valueHistory) {

        if (
            change.name == name &&
            change.line < line
        ) {

            state = change.state;
        }
    }

    return state;
}

bool SourceContext::isContainerKnownNonEmptyAt(
    const std::string& name,
    int line
) const {

    bool hasState = false;

    bool maybeEmpty = true;

    for (const auto& change : containerHistory) {

        if (
            change.name == name &&
            change.line < line
        ) {

            maybeEmpty = change.maybeEmpty;
            hasState = true;
        }
    }

    return hasState && !maybeEmpty;
}
