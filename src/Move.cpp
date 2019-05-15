#include "Move.h"

using CA3::Square;

Move::Move(Square _from, Square _to, MoveType _type)
        : from{_from}, to{_to}, type{_type} {

}

bool Move::isCapture() {
    return type == CAPTURE || type == EN_PASSANT || type >= PROMOTION_QUEEN_CAPTURE;
}

bool Move::isPromotion() {
    return type >= PROMOTION_QUEEN;
}

