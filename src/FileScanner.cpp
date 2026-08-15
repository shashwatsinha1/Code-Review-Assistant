#include "FileScanner.h"

#include <filesystem>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;

std::vector<std::string> FileScanner::findCppFiles(
    const std::string& path
) const {

    std::vector<std::string> files;

    fs::path inputPath(path);

    if (!fs::exists(inputPath)) {

        throw std::runtime_error(
            "Input path does not exist: " + path
        );
    }

    // Single file
    if (fs::is_regular_file(inputPath)) {

        std::string extension =
            inputPath.extension().string();

        if (extension == ".cpp" ||
            extension == ".cc" ||
            extension == ".cxx") {

            files.push_back(inputPath.string());
        }

        return files;
    }

    // Directory
    if (fs::is_directory(inputPath)) {

        for (const auto& entry :
             fs::recursive_directory_iterator(
                 inputPath,
                 fs::directory_options::skip_permission_denied
             )) {

            if (!entry.is_regular_file()) {
                continue;
            }

            // Skip unwanted directories
            bool ignored = false;

            for (const auto& part : entry.path()) {

                std::string directory =
                    part.string();

                if (directory == "build" ||
                    directory == ".git" ||
                    directory == ".vscode") {

                    ignored = true;
                    break;
                }
            }

            if (ignored) {
                continue;
            }

            std::string extension =
                entry.path().extension().string();

            if (extension == ".cpp" ||
                extension == ".cc" ||
                extension == ".cxx") {

                files.push_back(
                    entry.path().string()
                );
            }
        }
    }

    // Make file order deterministic
    std::sort(
        files.begin(),
        files.end()
    );

    return files;
}
