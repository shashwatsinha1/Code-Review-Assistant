#pragma once

#include <memory>
#include <vector>

#include "Rule.h"

class RuleFactory {

public:

    static std::vector<std::unique_ptr<Rule>>
    createDefaultRules();
};