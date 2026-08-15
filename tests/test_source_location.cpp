#include "SourceLocationInfo.h"

#include <cassert>

int main() {
    SourceLocationInfo location;

    assert(!location.isValid());

    location.file = "example.cpp";
    location.line = 10;
    location.column = 5;

    assert(location.isValid());
    assert(location.file == "example.cpp");
    assert(location.line == 10);
    assert(location.column == 5);

    location.line = 0;

    assert(!location.isValid());

    return 0;
}