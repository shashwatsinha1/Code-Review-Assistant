#include <iostream>
#include <string>
#include <vector>

#include "ReviewService.h"
#include "ReviewReport.h"

int main(int argc, char* argv[]) {
    
    if (argc < 2) {

        std::cerr
            << "Usage: CodeReviewAssistant <file-or-directory>\n";

        return 1;
    }

    std::string inputPath = argv[1];

    ReviewService service;

    std::vector<ReviewFinding> findings =
        service.review(inputPath);

    ReviewReport report(findings);

    report.print();

    report.saveJson(
        "review_report.json"
    );

    return 0;
}