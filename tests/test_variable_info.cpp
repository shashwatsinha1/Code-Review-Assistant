#include "VariableInfo.h"

#include <cassert>

int main() {
    VariableInfo variable;

    assert(variable.name.empty());
    assert(variable.type.empty());
    assert(!variable.isLocal);
    assert(!variable.isParameter);
    assert(!variable.isField);
    assert(!variable.hasInitializer);
    assert(!variable.isUsed);

    variable.name = "count";
    variable.type = "int";

    variable.location.file = "example.cpp";
    variable.location.line = 10;
    variable.location.column = 9;

    variable.isLocal = true;
    variable.hasInitializer = true;
    variable.isUsed = true;

    assert(variable.name == "count");
    assert(variable.type == "int");
    assert(variable.location.isValid());
    assert(variable.isLocal);
    assert(!variable.isParameter);
    assert(!variable.isField);
    assert(variable.hasInitializer);
    assert(variable.isUsed);

    return 0;
}