#include "RuleFactory.h"

#include "DivisionRule.h"
#include "BoundsRule.h"
#include "EmptyContainerRule.h"
#include "ComplexityRule.h"
#include "UninitializedRule.h"
#include "UnusedVariableRule.h"
#include "NullPointerRule.h"
#include "MemoryLeakRule.h"

std::vector<std::unique_ptr<Rule>>
RuleFactory::createDefaultRules() {

    std::vector<std::unique_ptr<Rule>> rules;

    rules.push_back(
        std::make_unique<BoundsRule>()
    );

    rules.push_back(
        std::make_unique<EmptyContainerRule>()
    );

    rules.push_back(
        std::make_unique<ComplexityRule>()
    );

    rules.push_back(
        std::make_unique<DivisionRule>()
    );

    rules.push_back(
        std::make_unique<UninitializedRule>()
    );

    rules.push_back(
        std::make_unique<UnusedVariableRule>()
    );

    rules.push_back(
        std::make_unique<NullPointerRule>()
    );

    rules.push_back(
        std::make_unique<MemoryLeakRule>()
    );
    
    return rules;   
}
