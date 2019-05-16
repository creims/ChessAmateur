#include "catch.hpp"
#include "../src/Move.h"

TEST_CASE("Test isCapture") {
    for(int t = MOVE; t <= PROMOTION_KNIGHT_CAPTURE; t++) {
        auto type = static_cast<MoveType>(t);
        Move m = {0, 0, type};
        switch(type) {
            case CAPTURE:
            case EN_PASSANT:
            case PROMOTION_QUEEN_CAPTURE:
            case PROMOTION_ROOK_CAPTURE:
            case PROMOTION_BISHOP_CAPTURE:
            case PROMOTION_KNIGHT_CAPTURE:
                REQUIRE(m.isCapture());
                break;
            default:
                REQUIRE(!m.isCapture());
        }
    }
}

