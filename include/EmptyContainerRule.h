#pragma once

#include "Rule.h"

class EmptyContainerRule : public Rule{
public:
    
    std::string getId() const override;

    std::string getName() const override;

    std::vector<ReviewFinding> check(
        const SourceFile& source,
        const SourceContext& context
    ) override;
};