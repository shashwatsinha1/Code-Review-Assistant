#include "MemoryLeakRule.h"

#include <regex>
#include <unordered_map>

struct AllocationInfo {
    int line;
    std::string code;
    bool arrayAllocation;
    bool deleted;
};

std::string MemoryLeakRule::getId() const {
    return "M001";
}

std::string MemoryLeakRule::getName() const {
    return "Possible Memory Leak";
}

std::vector<ReviewFinding> MemoryLeakRule::check(
    const SourceFile& source,
    const SourceContext& context
) {

    (void)context;

    std::vector<ReviewFinding> findings;

    const auto& lines =
        source.getLines();

    std::unordered_map<std::string, AllocationInfo> allocations;

    std::regex allocationPattern(
        R"(\b(?:[\w:<>]+)\s*\*\s*(\w+)\s*=\s*new\b[^;]*(\[[^\]]*\])?\s*;)"
    );

    std::regex deletePattern(
        R"(\bdelete\s*(\[\s*\])?\s*(\w+)\s*;)"
    );

    for (size_t i = 0; i < lines.size(); i++) {

        const std::string& line =
            lines[i];

        std::smatch match;

        if (
            std::regex_search(
                line,
                match,
                allocationPattern
            )
        ) {

            AllocationInfo allocation;
            allocation.line = static_cast<int>(i + 1);
            allocation.code = line;
            allocation.arrayAllocation = match[2].matched;
            allocation.deleted = false;

            allocations[match[1].str()] = allocation;
            continue;
        }

        if (
            std::regex_search(
                line,
                match,
                deletePattern
            )
        ) {

            auto it =
                allocations.find(match[2].str());

            if (it != allocations.end()) {
                it->second.deleted = true;
            }
        }
    }

    for (const auto& entry : allocations) {

        if (entry.second.deleted) {
            continue;
        }

        ReviewFinding finding;
        finding.ruleId = getId();
        finding.severity = Severity::MEDIUM;
        finding.category = Category::MEMORY;
        finding.line = entry.second.line;
        finding.code = entry.second.code;
        finding.title =
            "Possible memory leak";
        finding.description =
            "Memory allocated for '" +
            entry.first +
            "' is not released with delete.";
        finding.suggestion =
            "Release '" +
            entry.first +
            "' with delete or prefer RAII ownership.";

        findings.push_back(finding);
    }

    return findings;
}
