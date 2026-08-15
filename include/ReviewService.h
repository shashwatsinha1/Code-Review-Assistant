#pragma once

#include <string>
#include <vector>

#include "ReviewFinding.h"
#include "RuleEngine.h"
#include "FileScanner.h"

class ReviewService {

private:

    FileScanner scanner;

    RuleEngine engine;

public:

    ReviewService();

    std::vector<ReviewFinding> review(
        const std::string& path
    );
};