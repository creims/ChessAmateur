#ifndef CHESSAMATEUR3_PIECE_H
#define CHESSAMATEUR3_PIECE_H

#include <stdint.h>

namespace CA3 {
    typedef uint8_t PieceCharacteristic;

    constexpr PieceCharacteristic PIECE_BLACK = 0b0000'0001;
    constexpr PieceCharacteristic PIECE_WHITE = 0b0000'0010;
    constexpr PieceCharacteristic PIECE_PAWN = 0b0000'0100;
    constexpr PieceCharacteristic PIECE_KNIGHT = 0b0000'1000;
    constexpr PieceCharacteristic PIECE_BISHOP = 0b0001'0000;
    constexpr PieceCharacteristic PIECE_ROOK = 0b0010'0000;
    constexpr PieceCharacteristic PIECE_QUEEN = 0b0100'0000;
    constexpr PieceCharacteristic PIECE_KING = 0b1000'0000;

    constexpr PieceCharacteristic MASK_COLOR = PIECE_WHITE | PIECE_BLACK;
    constexpr PieceCharacteristic MASK_PIECE = PIECE_PAWN | PIECE_KNIGHT | PIECE_BISHOP | PIECE_ROOK | PIECE_QUEEN | PIECE_KING;

    enum Color : uint8_t {
        BLACK = PIECE_BLACK,
        WHITE = PIECE_WHITE
    };

    enum Piece : uint8_t {
        NO_PIECE = 0,

        BLACK_PAWN = PIECE_BLACK | PIECE_PAWN,
        BLACK_KNIGHT = PIECE_BLACK | PIECE_KNIGHT,
        BLACK_BISHOP = PIECE_BLACK | PIECE_BISHOP,
        BLACK_ROOK = PIECE_BLACK | PIECE_ROOK,
        BLACK_QUEEN = PIECE_BLACK | PIECE_QUEEN,
        BLACK_KING = PIECE_BLACK | PIECE_KING,

        WHITE_PAWN = PIECE_WHITE | PIECE_PAWN,
        WHITE_KNIGHT = PIECE_WHITE | PIECE_KNIGHT,
        WHITE_BISHOP = PIECE_WHITE | PIECE_BISHOP,
        WHITE_ROOK = PIECE_WHITE | PIECE_ROOK,
        WHITE_QUEEN = PIECE_WHITE | PIECE_QUEEN,
        WHITE_KING = PIECE_WHITE | PIECE_KING
    };

    constexpr bool isPawn(Piece p) { return p & PIECE_PAWN; }

    constexpr bool isKnight(Piece p) { return p & PIECE_KNIGHT; }

    constexpr bool isBishop(Piece p) { return p & PIECE_BISHOP; }

    constexpr bool isRook(Piece p) { return p & PIECE_ROOK; }

    constexpr bool isQueen(Piece p) { return p & PIECE_QUEEN; }

    constexpr bool isKing(Piece p) { return p & PIECE_KING; }

    constexpr bool isEnemy(Piece p, Color c) { return p && !(c & p); }

    constexpr bool isFriendly(Piece p, Color c) { return c & p; }

    constexpr PieceCharacteristic pieceType(Piece p) { return p & MASK_PIECE; }

    constexpr Color pieceColor(Piece p) { return p & PIECE_BLACK ? BLACK : WHITE; }

    constexpr Color enemyColor(Color c) { return c == WHITE ? BLACK : WHITE; }

    constexpr bool oppositeColors(Piece p1, Piece p2) { return (p1 & MASK_COLOR) != (p2 & MASK_COLOR); }

}
#endif //CHESSAMATEUR3_PIECE_H
