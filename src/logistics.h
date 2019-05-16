#ifndef CHESSAMATEUR3_LOGISTICS_H
#define CHESSAMATEUR3_LOGISTICS_H

#include "piece.h"

// Contains definitions and functions for board squares and how pieces can move among them

// Example usage of directions: finding squares a queen can visit from square 40

// Square from = 40;
// for(Direction dir = ALLDIR_START; dir <= ALLDIR_END; ++dir) {
//   int inc = dirIncrement(dir);
//   Square inDir;
//   for(Square const* p = dirPtr(dir, s); (inDir = *p) != INVALID_SQUARE; p += inc) {
//     ... // inDir is visitable if not blocked
//   }
// }

namespace CA3 {
    typedef uint8_t Square;
    typedef uint8_t Direction;

    constexpr Square INVALID_SQUARE = 64;

    // Destination squares for castling - common to regular chess and Chess 960
    constexpr Square CASTLE_WEST_BLACK_KING = 2;
    constexpr Square CASTLE_WEST_BLACK_ROOK = 3;
    constexpr Square CASTLE_EAST_BLACK_KING = 6;
    constexpr Square CASTLE_EAST_BLACK_ROOK = 5;

    constexpr Square CASTLE_WEST_WHITE_KING = 58;
    constexpr Square CASTLE_WEST_WHITE_ROOK = 59;
    constexpr Square CASTLE_EAST_WHITE_KING = 62;
    constexpr Square CASTLE_EAST_WHITE_ROOK = 61;

    // Directions
    constexpr Direction SOUTH = 0;
    constexpr Direction NORTH = 1;
    constexpr Direction EAST = 2;
    constexpr Direction WEST = 3;
    constexpr Direction SOUTHEAST = 4;
    constexpr Direction NORTHWEST = 5;
    constexpr Direction SOUTHWEST = 6;
    constexpr Direction NORTHEAST = 7;
    constexpr Direction INVALID_DIRECTION = 8;

    // These are used to iterate through directions, eg from RANKFILE_START to RANKFILE_END for rook moves
    constexpr Direction RANKFILE_START = SOUTH;
    constexpr Direction RANKFILE_END = WEST;
    constexpr Direction DIAGONAL_START = SOUTHEAST;
    constexpr Direction DIAGONAL_END = NORTHEAST;
    constexpr Direction ALLDIR_START = SOUTH;
    constexpr Direction ALLDIR_END = NORTHEAST;

    // These are best accessed through functions: dirPtr, filePtr, rankPtr, knightPtr, kingPtr
    extern const int* indices[];
    extern const Square* squares[];

    extern const unsigned int knight_indices[];
    extern const Square knight_data[];

    extern const unsigned int king_indices[];
    extern const Square king_data[];

    // Get the increment to be used with a given direction
    // This is used to move forward or backward through data, as directions are complementary
    constexpr int dirIncrement(Direction d) { return d & 1u ? -1 : 1; }

    // Returns a pointer to the first square in the direction d from square s
    // Increment this pointer by dirIncrement(d) until its dereferenced value is INVALID_SQUARE
    constexpr Square const* dirPtr(Direction d, Square s) {
        return &squares[d >> 1u][indices[d][s]];
    }

    // Returns a pointer to the first element of a file in direction
    // For example, square 52 is in the e file, so filePtr(SOUTH, 52)
    // returns a pointer p such that *p = 4 (e8), *(p + 1) = 12 (e7), etc
    constexpr Square const* filePtr(Direction d, Square s) {
        return &squares[d >> 1u][indices[d][s % 8] - dirIncrement(d)];
    }

    // Returns a pointer to the first element of a rank in direction
    // For example, square 13 is in rank 7, so rankPtr(EAST, 13)
    // returns a pointer p such that *p = 8 (a7), *(p + 1) = 9 (b7), etc
    constexpr Square const* rankPtr(Direction d, Square s) {
        return &squares[d >> 1u][indices[d][s - (s % 8)] - dirIncrement(d)];
    }

    // Dereference and then increment this pointer until *p is INVALID_SQUARE in order to cycle through all possible
    // knight moves from s
    constexpr Square const* knightPtr(Square s) {
        return &knight_data[knight_indices[s]];
    }

    // Dereference and then increment this pointer until *p is INVALID_SQUARE in order to cycle through all possible
    // king moves from s
    constexpr Square const* kingPtr(Square s) {
        return &king_data[king_indices[s]];
    }

    // Other utility functions
    constexpr bool isDiagonal(Direction d) { return d != INVALID_DIRECTION && d > WEST; }

    constexpr bool isStraight(Direction d) { return d < SOUTHEAST; }

    constexpr bool outOfBounds(Square toCheck) { return toCheck < 0 || toCheck > 63; }

    constexpr bool inBounds(Square toCheck) { return toCheck >= 0 && toCheck < 64; }

    constexpr bool whiteHomeRow(Square s) { return s > 47 && s < 56; }

    constexpr bool blackHomeRow(Square s) { return s > 7 && s < 16; }

    constexpr bool onHomeRow(Square s, Color c) { return c == WHITE ? whiteHomeRow(s) : blackHomeRow(s); }

    constexpr bool onPromotionRow(Square s) { return s < 8 || s > 55; }

    // This is NOT the manhattan distance. For example, a7 and h8 only has a distance of 1 (8 vs 7, respectively)
    constexpr int distance(Square from, Square to) {
        int d = from - to;
        return d < 0 ? -d : d;
    }

    // Returns the horizontal distance along the board, eg h6 and g1 have a horizontal distance of 1
    constexpr int horizontalDistance(Square from, Square to) {
        int d = (from % 8) - (to % 8);
        return d < 0 ? -d : d;
    }

    // Returns vertical distance along the board, eg h1 and b3 has a vertical distance of 2
    constexpr int verticalDistance(Square from, Square to) {
        int d = (from / 8) - (to / 8);
        return d < 0 ? -d : d;
    }

    // Piece-related movement checks
    constexpr bool validKnightMovement(Square from, Square to) {
        // All valid knight moves have a distance of + or - 6, 10, 15, 17
        int dist = distance(from, to);

        // Vertical wraps ignored (bounds are always pre-checked), but do check for horizontal wraps
        return (dist == 6 || dist == 10 || dist == 15 || dist == 17) && horizontalDistance(from, to) < 3;
    }

    constexpr bool inKingRange(Square attackingSquare, Square defendingSquare) {
        return horizontalDistance(attackingSquare, defendingSquare) < 2 &&
               verticalDistance(attackingSquare, defendingSquare) < 2;
    }

    // Returns true if a pawn of attackerColor on attackingSquare can attack defendingSquare
    constexpr bool validPawnAttack(Square attackingSquare, Square defendingSquare, Color attackerColor) {
        if (attackerColor == WHITE) {
            return defendingSquare == attackingSquare - 7 || defendingSquare == attackingSquare - 9;
        } else {
            return defendingSquare == attackingSquare + 7 || defendingSquare == attackingSquare + 9;
        }
    }

    // Returns the square behind s, where behind is relative to a certain color (for en passant)
    constexpr Square squareBehind(Square s, Color c) {
        return c == WHITE ? s + 8 : s - 8;
    }

    // Returns the direction between two squares, or INVALID_DIR if it's none of N, E, S, W, NE, NW, SE, SW
    Direction getDirection(Square from, Square to);
}

#endif //CHESSAMATEUR3_LOGISTICS_H
