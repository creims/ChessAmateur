#include "catch.hpp"
#include "../src/logistics.h"

// Board reference:
// 0  1  2  3  4  5  6  7
// 8  9  10 11 12 13 14 15
// 16 17 18 19 20 21 22 23
// 24 25 26 27 28 29 30 31
// 32 33 34 35 36 37 38 39
// 40 41 42 43 44 45 46 47
// 48 49 50 51 52 53 54 55
// 56 57 58 59 60 61 62 63

using std::string;
using namespace CA3;

string squaresInDir(Direction dir, Square s) {
    string dirs;

    int inc = dirIncrement(dir);
    Square inDir;
    for(Square const* p = dirPtr(dir, s); (inDir = *p) != INVALID_SQUARE; p += inc) {
        if(!dirs.empty()) {
            dirs += " ";
        }
        dirs += std::to_string(inDir);
    }

    return dirs;
}

TEST_CASE("Test north/south") {
    REQUIRE(squaresInDir(NORTH, 34) == "26 18 10 2");
    REQUIRE(squaresInDir(SOUTH, 34) == "42 50 58");
}

TEST_CASE("Test east/west") {
    REQUIRE(squaresInDir(EAST, 41) == "42 43 44 45 46 47");
    REQUIRE(squaresInDir(WEST, 18) == "17 16");
}

TEST_CASE("Test southeast/northwest") {
    REQUIRE(squaresInDir(SOUTHEAST, 2) == "11 20 29 38 47");
    REQUIRE(squaresInDir(SOUTHEAST, 14) == "23");
    REQUIRE(squaresInDir(SOUTHEAST, 0) == "9 18 27 36 45 54 63");
    REQUIRE(squaresInDir(SOUTHEAST, 24) == "33 42 51 60");

    REQUIRE(squaresInDir(NORTHWEST, 53) == "44 35 26 17 8");
    REQUIRE(squaresInDir(NORTHWEST, 23) == "14 5");
    REQUIRE(squaresInDir(NORTHWEST, 25) == "16");
    REQUIRE(squaresInDir(NORTHWEST, 24).empty());
    REQUIRE(squaresInDir(NORTHWEST, 39) == "30 21 12 3");

    // corner cases (ha)
    REQUIRE(squaresInDir(SOUTHEAST, 0) == "9 18 27 36 45 54 63");
    REQUIRE(squaresInDir(SOUTHEAST, 7).empty());
    REQUIRE(squaresInDir(SOUTHEAST, 56).empty());
    REQUIRE(squaresInDir(SOUTHEAST, 63).empty());

    REQUIRE(squaresInDir(NORTHWEST, 0).empty());
    REQUIRE(squaresInDir(NORTHWEST, 7).empty());
    REQUIRE(squaresInDir(NORTHWEST, 56).empty());
    REQUIRE(squaresInDir(NORTHWEST, 63) == "54 45 36 27 18 9 0");

}

TEST_CASE("Test southwest/northeast") {
    REQUIRE(squaresInDir(SOUTHWEST, 27) == "34 41 48");
    REQUIRE(squaresInDir(SOUTHWEST, 55) == "62");
    REQUIRE(squaresInDir(SOUTHWEST, 4) == "11 18 25 32");
    REQUIRE(squaresInDir(SOUTHWEST, 31) == "38 45 52 59");

    REQUIRE(squaresInDir(NORTHEAST, 53) == "46 39");
    REQUIRE(squaresInDir(NORTHEAST, 5).empty());
    REQUIRE(squaresInDir(NORTHEAST, 43) == "36 29 22 15");
    REQUIRE(squaresInDir(NORTHEAST, 32) == "25 18 11 4");

    // corner cases (ha)
    REQUIRE(squaresInDir(SOUTHWEST, 0).empty());
    REQUIRE(squaresInDir(SOUTHWEST, 7) == "14 21 28 35 42 49 56");
    REQUIRE(squaresInDir(SOUTHWEST, 56).empty());
    REQUIRE(squaresInDir(SOUTHWEST, 63).empty());

    REQUIRE(squaresInDir(NORTHEAST, 0).empty());
    REQUIRE(squaresInDir(NORTHEAST, 7).empty());
    REQUIRE(squaresInDir(NORTHEAST, 56) == "49 42 35 28 21 14 7");
    REQUIRE(squaresInDir(NORTHEAST, 63).empty());
}
