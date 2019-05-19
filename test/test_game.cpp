#include "catch.hpp"

#include <iostream>
#include "../src/Game.h"

using namespace std;
using namespace CA3;

const string startingBoard = "rnbqkbnr"
                             "pppppppp"
                             "........"
                             "........"
                             "........"
                             "........"
                             "PPPPPPPP"
                             "RNBQKBNR";

TEST_CASE("lastMoveString") {
    Game g{};

    SECTION("Should throw an error if there is no last move") {
        REQUIRE_THROWS(g.lastMoveString());
    }

    SECTION("Moves") {
        SECTION("Should return eg 'e4' for normal pawn moves and forced marches") {
            g.tryMove(52, 36);
            REQUIRE(g.lastMoveString() == "e4");
            g.tryMove(8, 16);
            REQUIRE(g.lastMoveString() == "a6");
        }

        SECTION("Should return eg 'Ne6' or 'Kg5' for other piece moves") {
            g.tryMove(52, 36);
            g.tryMove(12, 28);

            g.tryMove(59, 31); // Move Q
            REQUIRE(g.lastMoveString() == "Qh5");

            g.tryMove(5, 40); // Move b
            REQUIRE(g.lastMoveString() == "Ba3");

            g.tryMove(57, 42); // Move N
            REQUIRE(g.lastMoveString() == "Nc3");

            g.tryMove(4, 5); // Move k
            REQUIRE(g.lastMoveString() == "Kf8");

            g.tryMove(56, 57); // Move R
            REQUIRE(g.lastMoveString() == "Rb1");
        }
    }

    SECTION("Captures") {
        SECTION("Should return eg 'dxc6' for pawn captures") {
            g.tryMove(52, 36);
            g.tryMove(11, 27);
            g.tryMove(36, 27);
            REQUIRE(g.lastMoveString() == "exd5");
        }

        SECTION("Should return eg 'Nxe6' for other captures") {
            const string captures = ". k r . . Q . ."
                                    "p p . . . . . p"
                                    ". . p . . . . ."
                                    ". . p . . B . ."
                                    ". n . . . . . ."
                                    "N . . . . . . ."
                                    "P . P . . . P P"
                                    "R . . K . . . R";

            g.setBoard(captures);

            g.tryMove(5, 26); // Q
            REQUIRE(g.lastMoveString() == "Qxc5");

            g.setActivePlayer(WHITE);
            g.tryMove(29, 2); // B
            REQUIRE(g.lastMoveString() == "Bxc8");

            g.tryMove(33, 48); // n
            REQUIRE(g.lastMoveString() == "Nxa2");

            g.tryMove(56, 48); // R
            REQUIRE(g.lastMoveString() == "Rxa2");

            g.tryMove(1, 2); // k
            REQUIRE(g.lastMoveString() == "Kxc8");
        }
    }

    SECTION("Castling") {
        const string castle = "r . . . k . . r"
                              "p p p p p p p p"
                              ". . . . . . . ."
                              ". . . . . . . ."
                              ". . . . . . . ."
                              ". . . . . . . ."
                              "P P P P P P P P"
                              "R . . . K . . R";

        g.setBoard(castle);

        SECTION("Kingside castling returns O-O") {
            g.tryMove(60, 63);
            REQUIRE(g.lastMoveString() == "O-O");
            g.tryMove(4, 7);
            REQUIRE(g.lastMoveString() == "O-O");
        }

        SECTION("Queenside castling returns O-O-O") {
            g.tryMove(60, 56);
            REQUIRE(g.lastMoveString() == "O-O-O");
            g.tryMove(4, 0);
            REQUIRE(g.lastMoveString() == "O-O-O");
        }
    }

    SECTION("Non-pawn piece moves should return eg Qe2") {
        SECTION("Should return eg 'Nxe6' for other captures") {
            const string moves = ". k r . . Q . ."
                                 "p p . . . . . p"
                                 ". . p . . . . ."
                                 ". . . p . B . ."
                                 ". n . . . . . ."
                                 "N . . . . . . ."
                                 "P . P . . . P P"
                                 "R . K . . . . R";

            g.setBoard(moves);
            g.tryMove(29, 22); // B
            REQUIRE(g.lastMoveString() == "Bg6");

            g.tryMove(33, 16); // n
            REQUIRE(g.lastMoveString() == "Na6");

            g.tryMove(56, 57); // R
            REQUIRE(g.lastMoveString() == "Rb1");

            g.setActivePlayer(WHITE);
            g.tryMove(5, 3); // Q
            REQUIRE(g.lastMoveString() == "Qd8");

            g.tryMove(1, 0); // k
            REQUIRE(g.lastMoveString() == "Ka8");
        }
    }

    SECTION("Promotions should end with the piece in question") {
        const string promotions = ". k r . n . n ."
                                  "p p . . . P . p"
                                  ". . p . . . . ."
                                  ". . . p . B . ."
                                  ". n . . . . . ."
                                  "N . . . . . . ."
                                  "P . P . . p . P"
                                  "R . K . R . R R";

        g.setBoard(promotions);

        SECTION("Regular promotion returns eg 'f8=Q'") {
            g.makeMove(Move{13, 5, PROMOTION_ROOK});
            REQUIRE(g.lastMoveString() == "f8=R");

            g.makeMove(Move{53, 61, PROMOTION_BISHOP});
            REQUIRE(g.lastMoveString() == "f1=B");
        }

        SECTION("Capturing promotions return eg 'exf8=Q'") {
            g.makeMove(Move{13, 4, PROMOTION_KNIGHT_CAPTURE});
            REQUIRE(g.lastMoveString() == "fxe8=N");

            g.makeMove(Move{53, 62, PROMOTION_QUEEN_CAPTURE});
            REQUIRE(g.lastMoveString() == "fxg1=Q");
        }
    }

    SECTION("File and row ambiguity") {
        const string ambiguous = "k . r . n . n ."
                                 "p p . . . . P p"
                                 ". . p . n . . ."
                                 "B . B p . . Q Q"
                                 ". n . . . . Q ."
                                 ". . B . . . . ."
                                 "P . P . . p . P"
                                 "R . K . R . R R";

        g.setBoard(ambiguous);

        SECTION("When 2 pieces can move from the same file, the piece's rank is specified eg 'B3d3'") {
            g.tryMove(42, 35);
            REQUIRE(g.lastMoveString() == "B3d4");

            g.tryMove(4, 14);
            REQUIRE(g.lastMoveString() == "N8xg7"); // Takes is maintained
        }

        SECTION("When 2 pieces can move from the same rank, the piece's file is specified eg 'Nef6'") {
            g.tryMove(24, 33);
            REQUIRE(g.lastMoveString() == "Baxb4"); // Takes maintained

            g.tryMove(6, 21);
            REQUIRE(g.lastMoveString() == "Ngf6");
        }

        SECTION("In the rare case that both rank and file must be disambiguated: 'Qa1b5'") {
            g.tryMove(30, 39);
            REQUIRE(g.lastMoveString() == "Qg5h4");

            g.setActivePlayer(WHITE);
            g.tryMove(26, 33);
            REQUIRE(g.lastMoveString() == "Bc5xb4"); // Takes is maintained

        }
    }

    SECTION("Checks are appended with a +") {
        const string checks = ". k . . . . n ."
                              "p p . . . P . p"
                              ". . p . . B . ."
                              ". . . p . B . ."
                              ". n . . . . . ."
                              "N . . . . . . ."
                              "P . P . . p . p"
                              "R . K . . . R R";
        g.setBoard(checks);

        g.tryMove(21, 28);
        REQUIRE(g.lastMoveString() == "Be5+");

        // Get king out of check and return to white move
        g.tryMove(1, 0);
        g.tryMove(28, 21);

        // Pawn promote to queen with a check
        g.tryMove(53, 62);
        g.promote(QUEEN);
        REQUIRE(g.lastMoveString() == "fxg1=Q+");
    }

    SECTION("End of game") {
        SECTION("Checkmate should be indicated with # and followed by a player indication") {
            const string onemove = "rb.q.bnr"
                                   "....ppp."
                                   "pp......"
                                   "kp.....q"
                                   "n......."
                                   "........"
                                   "PPPPPPPP"
                                   ".RBQ.BNK"; // Black wins with 31 -> 55, white with 49->33

            g.setBoard(onemove);

            SECTION("White victory") {
                g.tryMove(49, 33); // White checkmates
                REQUIRE(g.lastMoveString() == "b4# 1-0");
            }

            SECTION("Black victory") {
                g.setActivePlayer(BLACK);
                g.tryMove(31, 55);
                REQUIRE(g.lastMoveString() == "Qxh2# 0-1");
            }
        }

        SECTION("Stalemates should be indicated with a ½–½") {
            const string stalemate = "k......."
                                     "........"
                                     "........"
                                     "..Q....."
                                     "........"
                                     "........"
                                     "........"
                                     ".......K"; // White causes stalemate with 26->10

            g.setBoard(stalemate);
            g.tryMove(26, 10);
            REQUIRE(g.lastMoveString() == "Qc7 ½–½");
        }
    }
}

TEST_CASE("New Game should reset the game") {
    Game g{};

    g.tryMove(55, 39);
    g.tryMove(6, 21);
    g.newGame();

    REQUIRE(g.getMoves().empty());
    REQUIRE(g.getBoard() == startingBoard);
}

TEST_CASE("Board set and get functions", "") {
    Game g{};

    const string differentBoard = "rnb.kbnr"
                                  "ppp.pppp"
                                  "........"
                                  "...p...."
                                  "........"
                                  "........"
                                  "PPPPPPPP"
                                  "RNBQKBNR";


    SECTION("Default board") {
        REQUIRE(g.getBoard() == startingBoard);
    }

    SECTION("Set board") {
        REQUIRE_NOTHROW(g.setBoard(differentBoard));
        REQUIRE(g.getBoard() == differentBoard);
    }


    SECTION("Invalid boards") {
        const string invalidBoard_short = "rnbqk";

        const string invalidBoard_wrong = "rnbq-bnr"
                                          "pppppppp"
                                          "........"
                                          "........"
                                          "........"
                                          "........"
                                          "PPPPPPPP"
                                          "RNBQKBNR";


        REQUIRE_THROWS(g.setBoard(invalidBoard_short));
        REQUIRE_THROWS(g.setBoard(invalidBoard_wrong));

        const string invalidBoard_wkingMissing = "rnbqkbnr"
                                                 "pppppppp"
                                                 "........"
                                                 "........"
                                                 "........"
                                                 "........"
                                                 "PPPPPPPP"
                                                 "RNBQ.BNR";

        const string invalidBoard_bkingMissing = "rnbq.bnr"
                                                 "pppppppp"
                                                 "........"
                                                 "........"
                                                 "........"
                                                 "........"
                                                 "PPPPPPPP"
                                                 "RNBQKBNR";

        REQUIRE_THROWS(g.setBoard(invalidBoard_wkingMissing));
        REQUIRE_THROWS(g.setBoard(invalidBoard_bkingMissing));

        const string invalidBoard_wkingMult = "rnbqkbnr"
                                              "pppppppp"
                                              "........"
                                              "........"
                                              ".K......"
                                              "........"
                                              "PPPPPPPP"
                                              "RNBQKBNR";

        const string invalidBoard_bkingMult = "rnbqkbnr"
                                              "pppppppp"
                                              "........"
                                              "........"
                                              "......k."
                                              "........"
                                              "PPPPPPPP"
                                              "RNBQKBNR";

        REQUIRE_THROWS(g.setBoard(invalidBoard_wkingMult));
        REQUIRE_THROWS(g.setBoard(invalidBoard_bkingMult));
    }
}

// This must deal with potentially extraneous input
TEST_CASE("Test tryMove", "") {
    Game g{};

    SECTION("Invalid board spaces (< 0 or > 63) throw an exception") {
        REQUIRE_THROWS(g.tryMove(-1, 0));
        REQUIRE_THROWS(g.tryMove(1, -1));
        REQUIRE_THROWS(g.tryMove(64, 0));
        REQUIRE_THROWS(g.tryMove(1, 64));
    }

    SECTION("Invalid moves throw an exception") {
        SECTION("Invalid pawn moves throw an exception") {
            const string pawns = "rnbqkbnr"
                                 "...p.ppp"
                                 "P..p..P."
                                 ".....P.P"
                                 "p.p....."
                                 ".p.P...p"
                                 "PPPP...."
                                 "RNBQKBNR";

            g.setBoard(pawns);

            // White pawn moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(49, 41)); // Blocked by p
            REQUIRE_THROWS(g.tryMove(48, 32)); // Blocked by p
            REQUIRE_THROWS(g.tryMove(51, 43)); // Blocked by P
            REQUIRE_THROWS(g.tryMove(51, 35)); // Forced march blocked by P
            REQUIRE_THROWS(g.tryMove(43, 27)); // Forced march impossible from non-starting row
            REQUIRE_THROWS(g.tryMove(50, 43)); // Can't take own piece
            REQUIRE_THROWS(g.tryMove(31, 22)); // Can't take own piece
            REQUIRE_THROWS(g.tryMove(48, 47)); // Can't capture by wrapping across the board
            REQUIRE_THROWS(g.tryMove(15, 23)); // Can't move opponent piece
            REQUIRE_THROWS(g.tryMove(22, 30)); // Can't move backward

            // Black pawn moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(11, 19)); // Blocked by p
            REQUIRE_THROWS(g.tryMove(14, 22)); // Blocked by P
            REQUIRE_THROWS(g.tryMove(41, 49)); // Blocked by P
            REQUIRE_THROWS(g.tryMove(11, 27)); // Forced march blocked by p
            REQUIRE_THROWS(g.tryMove(19, 35)); // Forced march impossible from non-starting row
            REQUIRE_THROWS(g.tryMove(40, 49)); // Can't take own piece
            REQUIRE_THROWS(g.tryMove(42, 49)); // Can't take own piece
            REQUIRE_THROWS(g.tryMove(15, 16)); // Can't capture by wrapping across the board
            REQUIRE_THROWS(g.tryMove(48, 40)); // Can't move opponent piece
            REQUIRE_THROWS(g.tryMove(34, 26)); // Can't move backward
        }

        SECTION("Invalid knight moves throw an exception") {
            const string knights = "rnbqkbnr"
                                   "...p.ppp"
                                   "......n."
                                   "..N....."
                                   "....N..n"
                                   "........"
                                   "PPPP...."
                                   "RKBQ.B.N";

            g.setBoard(knights);

            // White knight moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(26, 35)); // Can't move diagonally
            REQUIRE_THROWS(g.tryMove(26, 8)); // Can't move diagonally even further
            REQUIRE_THROWS(g.tryMove(26, 25)); // Can't move horizontally
            REQUIRE_THROWS(g.tryMove(26, 31)); // Can't move horizontally even further
            REQUIRE_THROWS(g.tryMove(26, 36)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(63, 40)); // Can't wrap to other side of board
            REQUIRE_THROWS(g.tryMove(39, 54)); // Can't move opponent piece

            // Black knight moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(39, 48)); // Can't capture by wrapping across the board
            REQUIRE_THROWS(g.tryMove(39, 22)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(22, 7)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(39, 25)); // Can't move by wrapping
            REQUIRE_THROWS(g.tryMove(26, 20)); // Can't move opponent piece
        }

        SECTION("Invalid bishop moves throw an exception") {
            const string bishops = "r n b q k b n r"
                                   ". . . p . p p p"
                                   ". . . . . . b ."
                                   ". . B . . . . ."
                                   ". . . . B . . b"
                                   ". . . . . . . ."
                                   "P P P P . . . ."
                                   "R K B Q . B . N"; // b: 5, 22, 39; B: 26, 36, 58, 61

            g.setBoard(bishops);

            // White bishop moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(26, 25)); // Can't move horizontally
            REQUIRE_THROWS(g.tryMove(26, 42)); // Can't move vertically
            REQUIRE_THROWS(g.tryMove(36, 63)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(26, 55)); // Can't wrap to other side of board
            REQUIRE_THROWS(g.tryMove(61, 32)); // Can't wrap to other side of board
            REQUIRE_THROWS(g.tryMove(58, 44)); // Can't move through pieces
            REQUIRE_THROWS(g.tryMove(5, 12)); // Can't move opponent piece

            // Black bishop moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(22, 15)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(22, 23)); // Can't move horizontally
            REQUIRE_THROWS(g.tryMove(22, 30)); // Can't move vertically
            REQUIRE_THROWS(g.tryMove(39, 40)); // Can't wrap to other side
            REQUIRE_THROWS(g.tryMove(39, 32)); // Can't wrap to other side/move horizontally
            REQUIRE_THROWS(g.tryMove(5, 23)); // Can't move through pieces
            REQUIRE_THROWS(g.tryMove(26, 33)); // Can't move opponent piece
        }

        SECTION("Invalid rook moves throw an exception") {
            const string rooks = "r n b q . b k r"
                                 ". . . p . p p p"
                                 ". . . . . . r ."
                                 ". . R . . . . ."
                                 ". . . . R . . r"
                                 ". . . . . . . ."
                                 "P P P P . . . ."
                                 "R K B Q . B . N"; // r: 22, 39; R: 26, 36, 56

            g.setBoard(rooks);

            // White rook moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(26, 19)); // Can't move diagonally
            REQUIRE_THROWS(g.tryMove(26, 40)); // Can't move diagonally
            REQUIRE_THROWS(g.tryMove(26, 50)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(56, 0)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(56, 40)); // Can't move through a piece
            REQUIRE_THROWS(g.tryMove(22, 30)); // Can't move opponent piece

            // Black rook moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(22, 29)); // Can't move diagonally
            REQUIRE_THROWS(g.tryMove(39, 30)); // Can't move diagonally
            REQUIRE_THROWS(g.tryMove(22, 14)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(39, 32)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(0, 56)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(26, 25)); // Can't move opponent piece
        }

        SECTION("Invalid queen moves throw an exception") {
            const string queens = "q n b q . b k q"
                                  ". . . p . p p p"
                                  ". . . . . . q ."
                                  ". . Q . . . . ."
                                  ". . . . Q . . q"
                                  ". . . . . . . ."
                                  "P P P P . . . ."
                                  "Q K B Q . B . N"; // q: 22, 39; Q: 26, 36, 56

            g.setBoard(queens);

            // White queen moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(26, 50)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(56, 0)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(56, 40)); // Can't move through a piece
            REQUIRE_THROWS(g.tryMove(22, 30)); // Can't move opponent piece

            // Black queen moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(22, 14)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(39, 32)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(0, 56)); // Can't wrap to other side of board/through a piece
            REQUIRE_THROWS(g.tryMove(26, 25)); // Can't move opponent piece
        }

        SECTION("Invalid king moves throw an exception") {
            const string kings = "q n b q . b n q"
                                 ". . . p . . p p"
                                 ". . . . . p . ."
                                 ". . n . k . . ."
                                 ". . . . P q . ."
                                 ". . . . . . . ."
                                 "P P P P . . . ."
                                 ". . B Q . N N K"; // K: 63, k: 28

            g.setBoard(kings);

            // White queen moves
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(63, 62)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(63, 56)); // Can't wrap to other side of board
            REQUIRE_THROWS(g.tryMove(63, 7)); // Can't wrap to other side of board
            REQUIRE_THROWS(g.tryMove(63, 55)); // Can't move into check
            REQUIRE_THROWS(g.tryMove(28, 27)); // Can't move opponent piece

            // Black queen moves
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(28, 37)); // Can't move onto own piece
            REQUIRE_THROWS(g.tryMove(28, 29)); // Can't move into check
            REQUIRE_THROWS(g.tryMove(63, 25)); // Can't move opponent piece
        }

        SECTION("Invalid castling moves throw an exception") {
            const string piecesInWay = "r n b q k b n r"
                                       "p p p p p p p p"
                                       ". . . . . . . ."
                                       ". . . . . . . ."
                                       ". . . . . . . ."
                                       ". . . . . . . ."
                                       "P P P P P P P P"
                                       "R N B Q K B N R";

            g.setBoard(piecesInWay);
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(60, 63));
            REQUIRE_THROWS(g.tryMove(60, 56));

            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(4, 7));
            REQUIRE_THROWS(g.tryMove(4, 0));

            const string underThreat = "r . . . k . . r"
                                       "p p p . p p p p"
                                       ". . B . . . . ."
                                       ". . . r . . r ."
                                       ". . . . . . . ."
                                       ". . . . . . . ."
                                       "P P P . P P . P"
                                       "R . . . K . . R";

            g.setBoard(underThreat);

            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(60, 63)); // Destination under threat
            REQUIRE_THROWS(g.tryMove(60, 56)); // Interim square under threat

            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(4, 0)); // Currently in check
        }

        SECTION("Kingside castling works") {
            const string castleOK = "r . . . k . . r"
                                    "p p p p p p p p"
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    "P P P P P P P P"
                                    "R . . . K . . R";

            const string afterone = "r....rk."
                                    "pppppppp"
                                    "........"
                                    "........"
                                    "........"
                                    "........"
                                    "PPPPPPPP"
                                    "R....RK.";

            g.setBoard(castleOK);
            g.setActivePlayer(WHITE);
            REQUIRE_NOTHROW(g.tryMove(60, 63)); // castle kingside
            REQUIRE_NOTHROW(g.tryMove(4, 7)); // castle kingside
            REQUIRE(g.getBoard() == afterone);
        }

        SECTION("Rooks moving from and then back to original should still invalidate castle") {
            const string castleOK = "r . . . k . . r"
                                    "p p p p p p p p"
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    "P P P P P P P P"
                                    "R . . . K . . R";

            g.setBoard(castleOK);
            g.tryMove(63, 62);
            g.setActivePlayer(WHITE);
            g.tryMove(62, 63);
            g.setActivePlayer(WHITE);
            REQUIRE_THROWS(g.tryMove(60, 63));

            g.setBoard(castleOK);
            g.setActivePlayer(BLACK);
            g.tryMove(7, 6);
            g.setActivePlayer(BLACK);
            g.tryMove(6, 7);
            g.setActivePlayer(BLACK);
            REQUIRE_THROWS(g.tryMove(4, 7));
        }
    }

    SECTION("Valid castling moves work") {
        SECTION("Kingside castling works") {
            const string castleOK = "r . . . k . . r"
                                    "p p p p p p p p"
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    "P P P P P P P P"
                                    "R . . . K . . R";

            const string afterone = "r....rk."
                                    "pppppppp"
                                    "........"
                                    "........"
                                    "........"
                                    "........"
                                    "PPPPPPPP"
                                    "R....RK.";

            g.setBoard(castleOK);
            g.setActivePlayer(WHITE);
            REQUIRE_NOTHROW(g.tryMove(60, 63)); // white, rook target
            REQUIRE_NOTHROW(g.tryMove(4, 6)); // black, empty square target
            REQUIRE(g.getBoard() == afterone);
        }

        SECTION("Queenside castling works") {
            const string castleOK = "r . . . k . . r"
                                    "p p p p p p p p"
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    ". . . . . . . ."
                                    "P P P P P P P P"
                                    "R . . . K . . R";

            const string afterone = "..kr...r"
                                    "pppppppp"
                                    "........"
                                    "........"
                                    "........"
                                    "........"
                                    "PPPPPPPP"
                                    "..KR...R";

            g.setBoard(castleOK);
            g.setActivePlayer(WHITE);
            REQUIRE_NOTHROW(g.tryMove(60, 58)); // white, empty square target
            REQUIRE_NOTHROW(g.tryMove(4, 0)); // black, rook target
            REQUIRE(g.getBoard() == afterone);
        }
    }

    SECTION("When game is over, tryMove should return WHITE_WINS or BLACK_WINS") {
        const string onemove = "rb.q.bnr"
                               "....ppp."
                               "pp......"
                               "kp.....q"
                               "n......."
                               "........"
                               "PPPPPPPP"
                               ".RBQ.BNK"; // Black wins with 31 -> 55, white with 49->33

        g.setBoard(onemove);
        g.setActivePlayer(WHITE);

        REQUIRE(g.tryMove(49, 33) == WHITE_WINS);

        g.setBoard(onemove);
        g.setActivePlayer(BLACK);
        REQUIRE(g.tryMove(31, 55) == BLACK_WINS);

        const string another = ". k r . . Q . ."
                               "p p . . . . . p"
                               ". . p . . . . ."
                               ". . . p . B . ."
                               ". n . . . . . ."
                               "N . . . . . . ."
                               "P . P . . . P P"
                               "R . K . . . . R";
        g.newGame();
        g.setBoard(another);

        REQUIRE(g.tryMove(5, 2) == WHITE_WINS);
    }

    SECTION("When a player can't move but isn't in check, tryMove should return STALEMATE") {
        const string stalemate = "k......."
                                 "........"
                                 "........"
                                 "..Q....."
                                 "........"
                                 "........"
                                 "........"
                                 ".......K"; // White causes stalemate with 26->10


        g.setBoard(stalemate);
        g.setActivePlayer(WHITE);

        REQUIRE(g.tryMove(26, 10) == STALEMATE);

        const string stalemate2 = "k......."
                                  "........"
                                  "........"
                                  "........"
                                  ".....n.b"
                                  ".......n"
                                  "........"
                                  ".......K"; // White causes stalemate with 39->46
        g.setBoard(stalemate2);
        g.setActivePlayer(BLACK);

        REQUIRE(g.tryMove(39, 46) == STALEMATE);
    }
}

TEST_CASE("Test valid moves", "") {
    Game g{};

    SECTION("Test pawn moves") {
        const string pawn_start = "rnbqkbnr"
                                  "pppppppp"
                                  "........"
                                  "........"
                                  "........"
                                  "........"
                                  "PPPPPPPP"
                                  "RNBQKBNR";
        g.setBoard(pawn_start);

        g.tryMove(48, 32); // WHITE force march
        g.tryMove(13, 29); // black force march
        g.tryMove(32, 24); // WHITE move
        g.tryMove(9, 25); // black force march
        g.tryMove(24, 17); // WHITE capture en passant
        g.tryMove(8, 17); // black capture
        g.tryMove(52, 44); // WHITE move
        g.tryMove(29, 37); // black move
        g.tryMove(54, 38); // WHITE force march
        g.tryMove(37, 46); // black capture en passant
        g.tryMove(44, 36); // WHITE move
        g.tryMove(46, 55); // black capture
        g.tryMove(36, 28); // WHITE move
        REQUIRE(g.tryMove(55, 62) == CHOOSE_PROMOTION); // black capture and promote
        g.promote(QUEEN);

        const string pawn_end = "rnbqkbnr"
                                "..ppp.pp"
                                ".p......"
                                "....P..."
                                "........"
                                "........"
                                ".PPP.P.."
                                "RNBQKBqR";

        REQUIRE(g.getBoard() == pawn_end);

        // Cover a rare case of en passant to get out of check
        const string epToDecheck = "rnbqkbnr"
                                   "..ppp.pp"
                                   ".p......"
                                   "....P..."
                                   "..K....."
                                   "........"
                                   ".PPP.P.."
                                   "RNBQ.BqR";
        g.setBoard(epToDecheck);

        g.setActivePlayer(BLACK);
        g.tryMove(11, 27);
        REQUIRE_NOTHROW(g.tryMove(28, 19));
    }
}