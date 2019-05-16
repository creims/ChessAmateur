#include <iostream>
#include "catch.hpp"

#include "../src/Move.h"
#include "../src/Game.h"
#include "../src/GameState.h"

using namespace CA3;

TEST_CASE("Test isThreatenedBy", "") {
    GameState gs;
    gs.makeEmpty();

    SECTION("Rook threats") {
        SECTION("Rook threats with no blockers") {
            // r R . . . . . .
            // . . . . . . . .
            // . . . . . . . .
            // . . . . . . . .
            // . . . . . . . .
            // . . . . . . . .
            // . . . . . . r .
            // . . . . . . . R
            gs[0] = BLACK_ROOK;
            gs[1] = WHITE_ROOK;
            gs[54] = BLACK_ROOK;
            gs[63] = WHITE_ROOK;

            REQUIRE(gs.isThreatenedBy(0, WHITE));
            REQUIRE(!gs.isThreatenedBy(1, WHITE));
            REQUIRE(gs.isThreatenedBy(2, WHITE));
            REQUIRE(gs.isThreatenedBy(7, WHITE));
            REQUIRE(!gs.isThreatenedBy(30, WHITE));
            REQUIRE(gs.isThreatenedBy(55, WHITE));
            REQUIRE(gs.isThreatenedBy(57, WHITE));

            REQUIRE(!gs.isThreatenedBy(0, BLACK));
            REQUIRE(gs.isThreatenedBy(1, BLACK));
            REQUIRE(!gs.isThreatenedBy(2, BLACK));
            REQUIRE(!gs.isThreatenedBy(7, BLACK));
            REQUIRE(gs.isThreatenedBy(30, BLACK));
            REQUIRE(gs.isThreatenedBy(55, BLACK));
            REQUIRE(gs.isThreatenedBy(56, BLACK));
            REQUIRE(!gs.isThreatenedBy(57, BLACK));
        }

        SECTION("Rook threats with blockers") {
            // r . . . p . . .
            // . R . . P . . .
            // . . . . . . . .
            // . . . . . . . .
            // P . . . . . . .
            // . p . . . . . .
            // . . . . . . . .
            // . . . . . . . .
            gs[0] = BLACK_ROOK;
            gs[9] = WHITE_ROOK;
            gs[4] = BLACK_PAWN;
            gs[12] = WHITE_PAWN;
            gs[32] = WHITE_PAWN;
            gs[41] = BLACK_PAWN;

            REQUIRE(gs.isThreatenedBy(3, BLACK));
            REQUIRE(gs.isThreatenedBy(4, BLACK)); // Yes, can threaten own piece (it's fine)
            REQUIRE(!gs.isThreatenedBy(5, BLACK));
            REQUIRE(gs.isThreatenedBy(24, BLACK));
            REQUIRE(gs.isThreatenedBy(32, BLACK));
            REQUIRE(!gs.isThreatenedBy(40, BLACK));

            REQUIRE(gs.isThreatenedBy(10, WHITE));
            REQUIRE(gs.isThreatenedBy(12, WHITE));
            REQUIRE(!gs.isThreatenedBy(15, WHITE));
            REQUIRE(gs.isThreatenedBy(33, WHITE));
            REQUIRE(gs.isThreatenedBy(41, WHITE));
            REQUIRE(!gs.isThreatenedBy(57, WHITE));
        }
    }

    SECTION("Bishop threats") {
        // . . . . . . . .
        // . . . . . . . .
        // . . . B . p . .
        // . . . . b . . .
        // . P . . . . . .
        // . . p . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[19] = WHITE_BISHOP;
        gs[21] = BLACK_PAWN;
        gs[28] = BLACK_BISHOP;
        gs[33] = WHITE_PAWN;
        gs[42] = BLACK_PAWN;

        REQUIRE(!gs.isThreatenedBy(0, WHITE));
        REQUIRE(gs.isThreatenedBy(1, WHITE));
        REQUIRE(gs.isThreatenedBy(10, WHITE));
        REQUIRE(gs.isThreatenedBy(28, WHITE));
        REQUIRE(gs.isThreatenedBy(12, WHITE));
        REQUIRE(gs.isThreatenedBy(5, WHITE));
        REQUIRE(!gs.isThreatenedBy(37, WHITE)); // blocked by b
        REQUIRE(!gs.isThreatenedBy(55, WHITE)); // blocked by b
        REQUIRE(gs.isThreatenedBy(33, WHITE));
        REQUIRE(!gs.isThreatenedBy(40, WHITE)); // blocked by P

        REQUIRE(gs.isThreatenedBy(19, BLACK));
        REQUIRE(gs.isThreatenedBy(21, BLACK));
        REQUIRE(!gs.isThreatenedBy(10, BLACK)); // blocked by B
        REQUIRE(!gs.isThreatenedBy(7, BLACK)); // blocked by p
        REQUIRE(gs.isThreatenedBy(46, BLACK));
        REQUIRE(!gs.isThreatenedBy(56, BLACK)); // blocked by p
    }

    SECTION("Queen threats") {
        SECTION("Nothing threatened") {
            // k . . . . . . . k at 0
            // . . . . . . . .
            // . . . . Q . . . Q at 20
            // . . . . . . . .
            // . . . . . . . .
            // q . . . . . . . q at 40
            // . . . . . . . .
            // . . . . K . . .  K at 60
            gs[0] = BLACK_KING;
            gs[20] = WHITE_QUEEN;
            gs[40] = BLACK_QUEEN;
            gs[60] = WHITE_KING;

            REQUIRE(!gs.isThreatenedBy(0, WHITE));
            REQUIRE(!gs.isThreatenedBy(20, BLACK));
            REQUIRE(!gs.isThreatenedBy(40, WHITE));
            REQUIRE(!gs.isThreatenedBy(60, BLACK));
        }
        SECTION("All threatened") {
            // k . . . . . . . k at 0
            // . . . . . . . .
            // . . . q . . . . q at 19
            // . . . . . . . .
            // . . . . . . . .
            // Q . . . . . . . Q at 40
            // . . . . . . . .
            // . . . K . . . .  K at 59
            gs[20] = NO_PIECE;
            gs[60] = NO_PIECE;
            gs[19] = BLACK_QUEEN;
            gs[40] = WHITE_QUEEN;
            gs[59] = BLACK_KING;

            REQUIRE(gs.isThreatenedBy(0, WHITE));
            REQUIRE(gs.isThreatenedBy(19, WHITE));
            REQUIRE(gs.isThreatenedBy(40, BLACK));
            REQUIRE(gs.isThreatenedBy(59, BLACK));
        }
    }

    SECTION("King threats") {
        // k . . . . . . . k at 0
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . K . . . . K at 35
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_KING;
        gs[35] = WHITE_KING;

        REQUIRE(gs.isThreatenedBy(1, BLACK));
        REQUIRE(gs.isThreatenedBy(9, BLACK));
        REQUIRE(gs.isThreatenedBy(8, BLACK));
        REQUIRE(!gs.isThreatenedBy(0, BLACK));
        REQUIRE(!gs.isThreatenedBy(36, BLACK));
        REQUIRE(!gs.isThreatenedBy(2, BLACK));

        REQUIRE(gs.isThreatenedBy(26, WHITE));
        REQUIRE(gs.isThreatenedBy(27, WHITE));
        REQUIRE(gs.isThreatenedBy(28, WHITE));
        REQUIRE(gs.isThreatenedBy(34, WHITE));
        REQUIRE(gs.isThreatenedBy(36, WHITE));
        REQUIRE(gs.isThreatenedBy(42, WHITE));
        REQUIRE(gs.isThreatenedBy(43, WHITE));
        REQUIRE(gs.isThreatenedBy(44, WHITE));
        REQUIRE(!gs.isThreatenedBy(35, WHITE));
        REQUIRE(!gs.isThreatenedBy(19, WHITE));
        REQUIRE(!gs.isThreatenedBy(8, WHITE));
    }

    SECTION("Pawn threats") {
        // . . . . . . . .
        // . P . . p . . .
        // . . p . . P . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[9] = WHITE_PAWN;
        gs[12] = BLACK_PAWN;
        gs[18] = BLACK_PAWN;
        gs[21] = WHITE_PAWN;

        REQUIRE(gs.isThreatenedBy(0, WHITE));
        REQUIRE(!gs.isThreatenedBy(1, WHITE));
        REQUIRE(gs.isThreatenedBy(2, WHITE));
        REQUIRE(!gs.isThreatenedBy(16, WHITE));
        REQUIRE(!gs.isThreatenedBy(17, WHITE));
        REQUIRE(!gs.isThreatenedBy(18, WHITE));

        REQUIRE(!gs.isThreatenedBy(9, BLACK));
        REQUIRE(!gs.isThreatenedBy(10, BLACK));
        REQUIRE(!gs.isThreatenedBy(11, BLACK));
        REQUIRE(gs.isThreatenedBy(25, BLACK));
        REQUIRE(!gs.isThreatenedBy(26, BLACK));
        REQUIRE(gs.isThreatenedBy(27, BLACK));
        REQUIRE(!gs.isThreatenedBy(36, BLACK));
        REQUIRE(!gs.isThreatenedBy(32, BLACK));

        REQUIRE(gs.isThreatenedBy(12, WHITE));
        REQUIRE(gs.isThreatenedBy(21, BLACK));
        REQUIRE(!gs.isThreatenedBy(6, WHITE));
    }

    SECTION("Knight threats") {
        // . . . . . . . .
        // . n . . N . . .
        // . . N . . n . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[9] = BLACK_KNIGHT;
        gs[12] = WHITE_KNIGHT;
        gs[18] = WHITE_KNIGHT;
        gs[21] = BLACK_KNIGHT;

        REQUIRE(!gs.isThreatenedBy(9, WHITE));
        REQUIRE(!gs.isThreatenedBy(12, BLACK));
        REQUIRE(!gs.isThreatenedBy(18, BLACK));
        REQUIRE(!gs.isThreatenedBy(21, WHITE));

        REQUIRE(gs.isThreatenedBy(24, BLACK));
        REQUIRE(gs.isThreatenedBy(26, BLACK));
        REQUIRE(gs.isThreatenedBy(19, BLACK));
        REQUIRE(gs.isThreatenedBy(3, BLACK));

        REQUIRE(gs.isThreatenedBy(12, WHITE));
        REQUIRE(gs.isThreatenedBy(18, WHITE));
        REQUIRE(gs.isThreatenedBy(35, WHITE));
        REQUIRE(gs.isThreatenedBy(1, WHITE));
        REQUIRE(gs.isThreatenedBy(33, WHITE));
    }
}

TEST_CASE("Test generateMoves", "") {
    GameState gs;
    gs.makeEmpty();

    SECTION("Generating pawn moves") {
        SECTION("Forward moves and forced march") {
            // . . . . . . . .
            // p . p . . . . .
            // . . . . . . P .
            // . . . . p . . .
            // . . . . . . p .
            // . . . . . . P .
            // . . . . P . . .
            // . . . . . . . .
            gs[8] = BLACK_PAWN;
            gs[10] = BLACK_PAWN;
            gs[28] = BLACK_PAWN;
            gs[38] = BLACK_PAWN;

            gs[22] = WHITE_PAWN;
            gs[46] = WHITE_PAWN;
            gs[52] = WHITE_PAWN;

            gs.setToAct(WHITE);
            REQUIRE(gs.generateMoves().size() == 3); // forced march and 2x move

            gs.setToAct(BLACK);
            REQUIRE(gs.generateMoves().size() == 5); // 2x forced, 3x move
        }
    }
    SECTION("Generating knight moves") {
        // n . . . N . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_KNIGHT;
        gs[4] = WHITE_KNIGHT;

        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 4);

        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 2);

        // Knight in the middle of the board
        gs[0] = NO_PIECE;
        gs[42] = BLACK_KNIGHT;
        REQUIRE(gs.generateMoves().size() == 8);
    }

    SECTION("Generating bishop moves") {
        // b . . . B . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . * . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_BISHOP;
        gs[4] = WHITE_BISHOP;

        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 7); // SW, SE

        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 7); // all SE

        // Bishop in the middle of the board (*)
        gs[0] = NO_PIECE;
        gs[42] = BLACK_BISHOP;
        REQUIRE(gs.generateMoves().size() == 11); // all directions
    }

    SECTION("Generating rook moves") {
        // r . . . R . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . * . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_ROOK;
        gs[4] = WHITE_ROOK;

        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 14); // can take and go W, E, S

        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 11); // Can take, 3 squares blocked

        // Rook in the middle of the board (*)
        gs[42] = BLACK_ROOK;
        REQUIRE(gs.generateMoves().size() == 25); // all directions + previous moves
    }

    SECTION("Generating queen moves") {
        // q . . . Q . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . * . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_QUEEN;
        gs[4] = WHITE_QUEEN;

        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 21); // can take and go W, E, S, SE, SW

        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 18); // Can take, 3 squares blocked

        // QUEEN in the middle of the board (*)
        gs[42] = BLACK_QUEEN;
        REQUIRE(gs.generateMoves().size() == 43); // all directions + previous moves
    }

    SECTION("Generate king moves") {
        // k . . . K . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        gs[0] = BLACK_KING;
        gs[4] = WHITE_KING;
        gs.setBlackKingLocation(0);
        gs.setWhiteKingLocation(4);

        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 5);

        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 3);

        // Move K to center of board and try
        gs[4] = NO_PIECE;
        gs[33] = WHITE_KING;
        gs.setToAct(WHITE);
        REQUIRE(gs.generateMoves().size() == 8);

        // Move k to center but 1 away from K and try
        gs[0] = NO_PIECE;
        gs[35] = BLACK_KING;
        gs.setToAct(BLACK);
        REQUIRE(gs.generateMoves().size() == 5); // k can't move left due to K

        gs[35] = NO_PIECE;
        gs[27] = BLACK_KING;
        REQUIRE(gs.generateMoves().size() == 6); // k can't move W or SW due to K
    }

    SECTION("Generate castling moves") {
        // r . . . k . . r
        // p . . . . . . p
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // P . . . . . . P
        // R . . . K . . R

        gs[0] = BLACK_ROOK;
        gs[4] = BLACK_KING;
        gs[7] = BLACK_ROOK;
        gs[8] = BLACK_PAWN;
        gs[15] = BLACK_PAWN;

        gs[48] = WHITE_PAWN;
        gs[55] = WHITE_PAWN;
        gs[56] = WHITE_ROOK;
        gs[60] = WHITE_KING;
        gs[63] = WHITE_ROOK;

        gs.setBlackKingLocation(4);
        gs.setWhiteKingLocation(60);
        gs.setBlackRookWestLocation(0);
        gs.setBlackRookEastLocation(7);
        gs.setWhiteRookWestLocation(56);
        gs.setWhiteRookEastLocation(63);

        // 4 pawn moves, 5 king moves, 5 rook moves, 2 castles = 16
        REQUIRE(gs.generateMoves().size() == 16);

        gs.makeMove({63, 62, MOVE});
        REQUIRE(gs.generateMoves().size() == 15); // black cannot castle kingside through check

        gs.makeMove({0, 2, MOVE});
        REQUIRE(gs.generateMoves().size() == 21); // white can no longer castle, but has 7 more rook moves
    }

    SECTION("Generating promotions moves") {
        // . r . . . . . .
        // P . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . p . . . . . .
        // N K . . . . . .

        gs[1] = BLACK_ROOK;
        gs[8] = WHITE_PAWN;
        gs[49] = BLACK_PAWN;
        gs[56] = WHITE_KNIGHT;
        gs[57] = WHITE_KING;

        // 4 promo moves (Q, N), 2 knight moves, 2 king moves = 8
        REQUIRE(gs.generateMoves().size() == 8);

        gs.setToAct(BLACK);
        // 12 rook moves, 2 promo moves = 14
        REQUIRE(gs.generateMoves().size() == 14);
    }
}
