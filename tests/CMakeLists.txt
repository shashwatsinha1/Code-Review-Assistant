function(add_code_review_test target source)
    add_executable(${target} ${source})

    target_link_libraries(
        ${target}
        PRIVATE
        CodeReviewCore
    )

    add_test(
        NAME ${target}
        COMMAND ${target}
    )
endfunction()

add_code_review_test(BoundsRuleTest test_bounds.cpp)
add_code_review_test(EmptyContainerRuleTest test_empty_container.cpp)
add_code_review_test(ComplexityRuleTest test_complexity.cpp)
add_code_review_test(DivisionRuleTest test_division.cpp)
add_code_review_test(UninitializedRuleTest test_uninitialized.cpp)
add_code_review_test(UnusedVariableRuleTest test_unused_variable.cpp)
add_code_review_test(NullPointerRuleTest test_null_pointer.cpp)
add_code_review_test(MemoryLeakRuleTest test_memory_leak.cpp)
add_code_review_test(CleanCodeTest test_clean.cpp)
add_code_review_test(SourceContextTest test_source_context.cpp)

add_executable(CLITest test_cli.cpp)

target_link_libraries(
    CLITest
    PRIVATE
    CodeReviewCore
)

target_compile_definitions(
    CLITest
    PRIVATE
    TEST_EXECUTABLE_PATH="$<TARGET_FILE:CodeReviewAssistant>"
    TEST_PROJECT_ROOT="${PROJECT_SOURCE_DIR}"
)

add_test(
    NAME CLITest
    COMMAND CLITest
)

set_tests_properties(
    CLITest
    PROPERTIES
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_code_review_test(
    ClangASTTest
    test_clang_ast.cpp
)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
    NAME WebAPITest
    COMMAND
        ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_web_api.py
        $<TARGET_FILE:CodeReviewWeb>
)

set_tests_properties(
    WebAPITest
    PROPERTIES
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(SourceLocationTest test_source_location.cpp)

target_link_libraries(
    SourceLocationTest
    PRIVATE
    CodeReviewCore
)

add_test(
    NAME SourceLocationTest
    COMMAND SourceLocationTest
)
