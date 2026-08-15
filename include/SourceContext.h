#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class ValueState {
    UNKNOWN,
    ZERO,
    NON_ZERO
};

struct VariableInfo {

    std::string name;

    std::string type;

    int line;
};

struct LoopInfo {

    std::string variable;

    std::string condition;

    int line;

    int depth;
};

struct ArrayAccessInfo {

    std::string container;

    std::string index;

    int line;
};

struct FunctionInfo {

    std::string name;

    int line;

    bool recursive;
};

struct SymbolInfo {

    std::string name;

    std::string type;

    int declarationLine;

    bool initialized;

    std::string value;

    int scopeDepth;

    ValueState valueState;

    bool maybeEmpty;
};

class SourceContext {

private:

    std::vector<VariableInfo> variables;

    std::vector<LoopInfo> loops;

    std::vector<ArrayAccessInfo> arrayAccesses;

    std::vector<FunctionInfo> functions;

    std::unordered_map<std::string, SymbolInfo> symbols;

    std::vector<
        std::unordered_map<std::string, SymbolInfo>
    > scopes;

    std::vector<SymbolInfo> allSymbols;

    struct ValueStateChange {
        std::string name;
        int line;
        ValueState state;
        std::string value;
    };

    struct ContainerStateChange {
        std::string name;
        int line;
        bool maybeEmpty;
    };

    std::vector<ValueStateChange> valueHistory;

    std::vector<ContainerStateChange> containerHistory;

public:

    explicit SourceContext(
        const std::string& code
    );

    const std::vector<VariableInfo>&
    getVariables() const;

    const std::vector<LoopInfo>&
    getLoops() const;

    const std::vector<ArrayAccessInfo>&
    getArrayAccesses() const;

    const std::vector<FunctionInfo>&
    getFunctions() const;

    const std::unordered_map<std::string, SymbolInfo>&
    getSymbols() const;

    const std::vector<
        std::unordered_map<std::string, SymbolInfo>
    >& getScopes() const;

    const std::vector<SymbolInfo>&
    getAllSymbols() const;

    ValueState getValueStateAt(
        const std::string& name,
        int line
    ) const;

    bool isContainerKnownNonEmptyAt(
        const std::string& name,
        int line
    ) const;
};
