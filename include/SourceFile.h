#pragma once

#include <string>
#include <vector>

class SourceFile {

private:

    std::string code;

    std::vector<std::string> lines;

public:

    explicit SourceFile(
        const std::string& code
    );

    const std::string& getCode() const;

    const std::vector<std::string>& getLines() const;

    const std::string& getLine(
        int lineNumber
    ) const;

    int lineCount() const;
};