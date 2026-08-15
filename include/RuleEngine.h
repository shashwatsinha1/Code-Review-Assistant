#pragma once

#include <memory>
#include <vector>

#include "Rule.h"
#include "ReviewFinding.h"
#include "SourceFile.h"

class RuleEngine {

private:

    std::vector<std::unique_ptr<Rule>> rules;

public:

    void addRule(
        std::unique_ptr<Rule> rule
    );

    std::vector<ReviewFinding> analyze(
        const SourceFile& source
    );
};