#pragma once

#include<vector>
#include<string>
#include "ReviewFinding.h"
#include "SourceFile.h"
#include "SourceContext.h"


class Rule{
public:
    virtual ~Rule() = default;

    virtual std::string getId() const = 0;

    virtual std::string getName() const = 0;

    virtual std::vector<ReviewFinding> check(
        const SourceFile& source,
        const SourceContext& context
    ) = 0;
};