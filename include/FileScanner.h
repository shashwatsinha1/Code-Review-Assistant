#pragma once

#include<string>
#include<vector>

class FileScanner{
public:
    std::vector<std::string> findCppFiles(
        const std::string& path
    ) const;
};