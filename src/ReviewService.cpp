#include "ReviewService.h"
#include "SourceFile.h"

#include <utility>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "RuleFactory.h"

ReviewService::ReviewService() {

    auto rules =
        RuleFactory::createDefaultRules();

    for (auto& rule : rules) {

        engine.addRule(
            std::move(rule)
        );
    }
}

std::vector<ReviewFinding> ReviewService::review(
    const std::string& path
) {

    std::vector<ReviewFinding> allFindings;

    std::vector<std::string> files =
        scanner.findCppFiles(path);

    if (files.empty()) {

        throw std::runtime_error(
            "No C++ source files found."
        );
    }

    for (const auto& filePath : files) {

        std::cout
            << "Analyzing: "
            << filePath
            << "\n";

        std::ifstream inputFile(filePath);

        if (!inputFile) {

            throw std::runtime_error(
                "Could not open source file: " +
                filePath
            );
        }

        std::string code(
            (std::istreambuf_iterator<char>(inputFile)),
            std::istreambuf_iterator<char>()
        );

        inputFile.close();

        // Create source representation
        SourceFile source(code);

        // Run review rules
        std::vector<ReviewFinding> findings =
            engine.analyze(source);

        for (auto& finding : findings) {

            finding.file = filePath;

            allFindings.push_back(
                finding
            );
        }
    }

    // Sort globally by filename and line number
    std::sort(
        allFindings.begin(),
        allFindings.end(),
        [](const ReviewFinding& a,
           const ReviewFinding& b) {

            if (a.file != b.file) {

                return a.file < b.file;
            }

            return a.line < b.line;
        }
    );

    return allFindings;
}
