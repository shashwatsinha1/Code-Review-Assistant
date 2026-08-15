#pragma once

#include <vector>
#include <string>

#include "ReviewFinding.h"

class ReviewReport {

private:
    std::vector<ReviewFinding> findings;
    
    void printSeveritySummary() const;

    void printRuleSummary() const;
public:

    ReviewReport(
        const std::vector<ReviewFinding>& findings
    );

    void print() const;

    int totalIssues() const;

    bool saveJson(
        const std::string& filename
    ) const;

    bool saveHtml(
        const std::string& filename
    ) const;
};
