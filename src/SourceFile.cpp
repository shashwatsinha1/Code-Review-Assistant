#include "SourceFile.h"

#include <sstream>
#include <stdexcept>

SourceFile::SourceFile(
    const std::string& code
)
    : code(code) {

    std::istringstream stream(code);

    std::string line;

    while (std::getline(stream, line)) {

        lines.push_back(line);
    }
}

const std::string& SourceFile::getCode() const {

    return code;
}

const std::vector<std::string>&
SourceFile::getLines() const {

    return lines;
}

const std::string& SourceFile::getLine(
    int lineNumber
) const {

    if (lineNumber < 1 ||
        lineNumber > static_cast<int>(lines.size())) {

        throw std::out_of_range(
            "Invalid source line number"
        );
    }

    return lines[lineNumber - 1];
}

int SourceFile::lineCount() const {

    return static_cast<int>(lines.size());
}