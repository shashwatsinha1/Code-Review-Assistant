#include "ReviewFinding.h"
#include "ReviewService.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

#ifdef _WIN32
using Socket = SOCKET;
constexpr Socket invalidSocket = INVALID_SOCKET;
#else
using Socket = int;
constexpr Socket invalidSocket = -1;
#endif

std::string severityToText(Severity severity) {
    switch (severity) {
        case Severity::INFO:
            return "INFO";
        case Severity::LOW:
            return "LOW";
        case Severity::MEDIUM:
            return "MEDIUM";
        case Severity::HIGH:
            return "HIGH";
        case Severity::CRITICAL:
            return "CRITICAL";
    }

    return "UNKNOWN";
}

std::string categoryToText(Category category) {
    switch (category) {
        case Category::BUG:
            return "BUG";
        case Category::PERFORMANCE:
            return "PERFORMANCE";
        case Category::MEMORY:
            return "MEMORY";
        case Category::SECURITY:
            return "SECURITY";
        case Category::STYLE:
            return "STYLE";
        case Category::LOGIC:
            return "LOGIC";
        case Category::BEST_PRACTICE:
            return "BEST_PRACTICE";
    }

    return "UNKNOWN";
}

json findingToJson(const ReviewFinding& finding) {
    return {
        {"rule", finding.ruleId},
        {"ruleId", finding.ruleId},
        {"severity", severityToText(finding.severity)},
        {"category", categoryToText(finding.category)},
        {"file", finding.file},
        {"line", finding.line},
        {"code", finding.code},
        {"title", finding.title},
        {"message", finding.title},
        {"description", finding.description},
        {"suggestion", finding.suggestion}
    };
}

json buildReviewResponse(const std::vector<ReviewFinding>& findings) {
    json response;
    response["totalIssues"] = findings.size();
    response["summary"] =
        std::to_string(findings.size()) +
        (findings.size() == 1 ? " issue found" : " issues found");
    response["findings"] = json::array();

    for (const auto& finding : findings) {
        response["findings"].push_back(findingToJson(finding));
    }

    return response;
}

std::string reasonPhrase(int statusCode) {
    switch (statusCode) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 500:
            return "Internal Server Error";
    }

    return "Error";
}

std::string httpResponse(
    int statusCode,
    const std::string& contentType,
    const std::string& body
) {
    std::ostringstream response;
    response
        << "HTTP/1.1 "
        << statusCode
        << " "
        << reasonPhrase(statusCode)
        << "\r\n"
        << "Content-Type: "
        << contentType
        << "\r\n"
        << "Content-Length: "
        << body.size()
        << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;

    return response.str();
}

std::string jsonResponse(int statusCode, const json& body) {
    return httpResponse(
        statusCode,
        "application/json",
        body.dump(4)
    );
}

std::string readTextFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Could not open static file.");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string contentTypeFor(const fs::path& path) {
    const std::string extension = path.extension().string();

    if (extension == ".html") {
        return "text/html; charset=utf-8";
    }

    if (extension == ".css") {
        return "text/css; charset=utf-8";
    }

    if (extension == ".js") {
        return "application/javascript; charset=utf-8";
    }

    return "text/plain; charset=utf-8";
}

std::string handleStaticFile(const std::string& path) {
    fs::path relativePath;

    if (path == "/" || path == "/index.html") {
        relativePath = "index.html";
    } else if (path.rfind("/static/", 0) == 0) {
        relativePath = fs::path(path.substr(1));
    } else {
        return jsonResponse(404, {{"error", "Route not found."}});
    }

    if (relativePath.string().find("..") != std::string::npos) {
        return jsonResponse(400, {{"error", "Invalid static path."}});
    }

    const fs::path filePath =
        fs::path(WEB_ROOT) / relativePath;

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        return jsonResponse(404, {{"error", "Static file not found."}});
    }

    try {
        return httpResponse(
            200,
            contentTypeFor(filePath),
            readTextFile(filePath)
        );
    } catch (const std::exception& error) {
        return jsonResponse(
            500,
            {{"error", "Could not serve static file."}, {"details", error.what()}}
        );
    }
}

std::string readRequest(Socket client) {
    std::string request;
    std::array<char, 4096> buffer{};
    std::size_t headerEnd = std::string::npos;
    int contentLength = 0;

    while (true) {
        const int received =
            recv(client, buffer.data(), static_cast<int>(buffer.size()), 0);

        if (received <= 0) {
            break;
        }

        request.append(buffer.data(), received);

        if (headerEnd == std::string::npos) {
            headerEnd = request.find("\r\n\r\n");

            if (headerEnd != std::string::npos) {
                const std::string headers = request.substr(0, headerEnd);
                const std::string key = "Content-Length:";
                const auto contentLengthPos = headers.find(key);

                if (contentLengthPos != std::string::npos) {
                    const auto valueStart = contentLengthPos + key.size();
                    const auto valueEnd =
                        headers.find("\r\n", valueStart);

                    contentLength = std::stoi(
                        headers.substr(valueStart, valueEnd - valueStart)
                    );
                }
            }
        }

        if (
            headerEnd != std::string::npos &&
            request.size() >= headerEnd + 4 + static_cast<std::size_t>(contentLength)
        ) {
            break;
        }
    }

    return request;
}

std::string requestBody(const std::string& request) {
    const auto headerEnd = request.find("\r\n\r\n");

    if (headerEnd == std::string::npos) {
        return "";
    }

    return request.substr(headerEnd + 4);
}

bool isBlank(const std::string& value) {
    return std::all_of(
        value.begin(),
        value.end(),
        [](unsigned char ch) {
            return std::isspace(ch);
        }
    );
}

std::vector<std::string> splitRequestLine(const std::string& request) {
    const auto lineEnd = request.find("\r\n");
    const std::string requestLine = request.substr(0, lineEnd);
    std::istringstream stream(requestLine);
    std::vector<std::string> parts;
    std::string part;

    while (stream >> part) {
        parts.push_back(part);
    }

    return parts;
}

fs::path writeTempSourceFile(const std::string& code) {
    const auto timestamp =
        std::chrono::steady_clock::now().time_since_epoch().count();

    const fs::path sourceFile =
        fs::temp_directory_path() /
        ("code_review_web_" + std::to_string(timestamp) + ".cpp");

    std::ofstream file(sourceFile);

    if (!file) {
        throw std::runtime_error("Could not create temporary source file.");
    }

    file << code;

    return sourceFile;
}

std::string handleReview(const std::string& body) {
    json request;

    try {
        request = json::parse(body);
    } catch (const std::exception& error) {
        return jsonResponse(
            400,
            {{"error", "Invalid JSON request."}, {"details", error.what()}}
        );
    }

    const std::string language = request.value("language", "cpp");

    if (language != "cpp" && language != "c++") {
        return jsonResponse(
            400,
            {{"error", "Only C++ review requests are supported."}}
        );
    }

    if (!request.contains("code") || !request["code"].is_string()) {
        return jsonResponse(
            400,
            {{"error", "Request body must include a string field named code."}}
        );
    }

    const std::string code = request["code"].get<std::string>();

    if (isBlank(code)) {
        return jsonResponse(
            400,
            {{"error", "Code must not be empty."}}
        );
    }

    const fs::path sourceFile =
        writeTempSourceFile(code);

    try {
        ReviewService service;
        const std::vector<ReviewFinding> findings =
            service.review(sourceFile.string());

        fs::remove(sourceFile);

        return jsonResponse(200, buildReviewResponse(findings));
    } catch (const std::exception& error) {
        fs::remove(sourceFile);

        return jsonResponse(
            500,
            {{"error", "Review failed."}, {"details", error.what()}}
        );
    }
}

std::string handleRequest(const std::string& request) {
    const std::vector<std::string> requestLine = splitRequestLine(request);

    if (requestLine.size() < 2) {
        return jsonResponse(400, {{"error", "Malformed HTTP request."}});
    }

    const std::string& method = requestLine[0];
    const std::string& path = requestLine[1];

    if (method == "OPTIONS") {
        return httpResponse(200, "text/plain", "");
    }

    if (method == "GET" && path == "/health") {
        return jsonResponse(200, {{"status", "ok"}});
    }

    if (method == "GET") {
        return handleStaticFile(path);
    }

    if (path != "/api/review") {
        return jsonResponse(404, {{"error", "Route not found."}});
    }

    if (method != "POST") {
        return jsonResponse(405, {{"error", "Method not allowed."}});
    }

    return handleReview(requestBody(request));
}

void closeSocket(Socket socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

bool initializeSockets() {
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
    return true;
#endif
}

void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

} // namespace

int main(int argc, char* argv[]) {
    const int port =
        argc >= 2 ? std::atoi(argv[1]) : 8080;

    if (!initializeSockets()) {
        std::cerr << "Failed to initialize sockets.\n";
        return 1;
    }

    Socket server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == invalidSocket) {
        std::cerr << "Failed to create server socket.\n";
        cleanupSockets();
        return 1;
    }

    const int enabled = 1;
    setsockopt(
        server,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&enabled),
        sizeof(enabled)
    );

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<unsigned short>(port));

    if (
        bind(
            server,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) < 0
    ) {
        std::cerr << "Failed to bind server to port " << port << ".\n";
        closeSocket(server);
        cleanupSockets();
        return 1;
    }

    if (listen(server, 16) < 0) {
        std::cerr << "Failed to listen for HTTP connections.\n";
        closeSocket(server);
        cleanupSockets();
        return 1;
    }

    std::cout
        << "CodeReviewWeb listening on http://127.0.0.1:"
        << port
        << "\n";

    while (true) {
        Socket client = accept(server, nullptr, nullptr);

        if (client == invalidSocket) {
            continue;
        }

        const std::string request = readRequest(client);
        const std::string response = handleRequest(request);

        send(
            client,
            response.c_str(),
            static_cast<int>(response.size()),
            0
        );

        closeSocket(client);
    }
}
