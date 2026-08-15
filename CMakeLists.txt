cmake_minimum_required(VERSION 3.16)

project(CodeReviewAssistant)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(json)

add_library(
    CodeReviewCore
    STATIC
    src/RuleEngine.cpp
    src/ReviewReport.cpp
    src/FileScanner.cpp
    src/rules/BoundsRule.cpp
    src/rules/EmptyContainerRule.cpp
    src/rules/ComplexityRule.cpp
    src/rules/DivisionRule.cpp
    src/rules/UninitializedRule.cpp
    src/rules/UnusedVariableRule.cpp
    src/rules/NullPointerRule.cpp
    src/rules/MemoryLeakRule.cpp
    src/ReviewService.cpp
    src/RuleFactory.cpp
    src/SourceFile.cpp
    src/SourceContext.cpp
    src/ClangASTParser.cpp
)

target_include_directories(
    CodeReviewCore
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    CodeReviewCore
    PUBLIC
    nlohmann_json::nlohmann_json
)

add_executable(
    CodeReviewAssistant
    src/main.cpp
)

target_link_libraries(
    CodeReviewAssistant
    PRIVATE
    CodeReviewCore
)

add_executable(
    CodeReviewWeb
    src/WebServer.cpp
)

target_link_libraries(
    CodeReviewWeb
    PRIVATE
    CodeReviewCore
)

target_compile_definitions(
    CodeReviewWeb
    PRIVATE
    WEB_ROOT="${PROJECT_SOURCE_DIR}/web"
)

if(WIN32)
    target_link_libraries(
        CodeReviewWeb
        PRIVATE
        ws2_32
    )
endif()

enable_testing()

add_subdirectory(tests)
