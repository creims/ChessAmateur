#ifndef CHESSAMATEUR3_MOVE_H
#define CHESSAMATEUR3_MOVE_H

#include "logistics.h"

enum MoveType : uint8_t {
    MOVE = 0, CAPTURE, FORCED_MARCH, EN_PASSANT, CASTLE_KINGSIDE, CASTLE_QUEENSIDE, NEED_PROMOTE,
    PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT,
    PROMOTION_QUEEN_CAPTURE, PROMOTION_ROOK_CAPTURE, PROMOTION_BISHOP_CAPTURE, PROMOTION_KNIGHT_CAPTURE
};

struct Move {
    Move() = default;

    Move(CA3::Square from, CA3::Square to, MoveType type);

    bool isCapture();
    bool isPromotion();

    CA3::Square from{}, to{};
    MoveType type{};
};


#endif //CHESSAMATEUR3_MOVE_H
