#include <cstring>

#include "GameState.h"
#include "logistics.h"

using namespace CA3;

// Returns 1 if x > 0, -1 if x < 0, and 0 if x == 0
constexpr int signOf(int x) {
    return (x > 0) - (x < 0);
}

// Separate validation and castling checks so we can generate errors
enum GameState::CastleResult : uint8_t {
    CASTLE_SUCCESS, CASTLE_ERR_MOVED, CASTLE_ERR_BLOCKED, CASTLE_ERR_KING_CHECK, CASTLE_ERR_SQUARE_CHECK
};

void GameState::makeEmpty() {
    memset(pieces, NO_PIECE, 64);
    whiteKingSquare = INVALID_SQUARE;
    whiteRookWest = INVALID_SQUARE;
    whiteRookEast = INVALID_SQUARE;
    blackKingSquare = INVALID_SQUARE;
    blackRookWest = INVALID_SQUARE;
    blackRookEast = INVALID_SQUARE;
    toAct = WHITE;
}

bool GameState::isBlocked(const Square from, const Square to, const Direction dir) const {
    Square checked;
    int inc = dirIncrement(dir);
    for (Square const* p = dirPtr(dir, from); (checked = *p) != INVALID_SQUARE; p += inc) {
        if (checked == to) {
            return false;
        }

        if (pieces[checked] != NO_PIECE) {
            return true;
        }
    }

    return false;
}

// Returns the nearest blocked square in a direction
Square GameState::nearestOccupiedInDir(Square from, Direction dir) const {
    int inc = dirIncrement(dir);
    Square to;

    for (Square const* p = dirPtr(dir, from); (to = *p) != INVALID_SQUARE; p += inc) {
        if (pieces[to] != NO_PIECE) {
            return to;
        }
    }

    return INVALID_SQUARE;
}

bool GameState::isThreatenedBySquare(Square defendingSquare, Square attackingSquare) const {
    Piece attackingPiece = pieces[attackingSquare];
    Color attackerColor = pieceColor(attackingPiece);

    PieceCharacteristic attackerType = pieceType(attackingPiece);

    if (attackerType == PIECE_PAWN) {
        return validPawnAttack(attackingSquare, defendingSquare, attackerColor);
    } else if (attackerType == PIECE_KING) {
        return inKingRange(attackingSquare, defendingSquare);
    } else if (attackerType == PIECE_KNIGHT) {
        return validKnightMovement(attackingSquare, defendingSquare);
    }

    Direction dir = getDirection(attackingSquare, defendingSquare);
    if (dir == INVALID_DIRECTION) {
        return false;
    }

    // If it's a straight direction, check for blocking pieces
    Square toCheck;
    int inc = dirIncrement(dir);

    for (Square const* p = dirPtr(dir, attackingSquare); (toCheck = *p) != INVALID_SQUARE; p += inc) {
        if (toCheck == defendingSquare) {
            if (isStraight(dir)) {
                return isQueen(attackingPiece) || isRook(attackingPiece);
            } else { // dir must be diagonal
                return isQueen(attackingPiece) || isBishop(attackingPiece);
            }
        }

        if (pieces[toCheck] != NO_PIECE) {
            return false;
        }
    }

    return false;
}

// Returns true if the square toCheck is threatened by enemyColor, and false otherwise.
bool GameState::isThreatenedBy(const Square toCheck, const Color enemyColor) const {
    // The plan: loop through the directions, finding the first potential attacker in that direction
    // Then check if the attacker can actually attack if it has limited range
    // Check knights separately, they can jump
    Square occupiedSquare;

    // Check N, E, S, W
    // Here we care about rooks, queens, and kings - nothing else can attack in these directions
    for (Direction dir = RANKFILE_START; dir <= RANKFILE_END; dir++) {
        occupiedSquare = nearestOccupiedInDir(toCheck, dir);

        if (occupiedSquare == INVALID_SQUARE) {
            continue;
        }

        Piece nearPiece = pieces[occupiedSquare];
        if (isFriendly(nearPiece, enemyColor)) {
            if (isRook(nearPiece) || isQueen(nearPiece)) {
                return true; // Rook or queen is threatening
            }

            if (isKing(nearPiece) && inKingRange(toCheck, occupiedSquare)) {
                return true; // King is threatening
            }
        }
    }

    // In order, check NE, NW, SE, SW
    // Here we care about pawns, bishops, queens, and kings - these can attack diagonally
    for (Direction dir = DIAGONAL_START; dir <= DIAGONAL_END; dir++) {
        occupiedSquare = nearestOccupiedInDir(toCheck, dir);
        if (occupiedSquare == INVALID_SQUARE) {
            continue;
        }

        Piece nearPiece = pieces[occupiedSquare];
        if (isFriendly(nearPiece, enemyColor)) {
            if (isBishop(nearPiece) || isQueen(nearPiece)) {
                return true; // Bishop or queen is threatening
            }

            // Kings can attack with range 1, and pawn attacks can attack in a subset of those situations
            if (inKingRange(toCheck, occupiedSquare)) {
                if (isKing(nearPiece)) {
                    return true;
                }

                if (isPawn(nearPiece) && validPawnAttack(occupiedSquare, toCheck, enemyColor)) {
                    return true;
                }
            } // End range 1 check
        }
    } // End diagonal check

    // Check knight threats
    // Iterate through potential knight attackers and judge if they're an enemy
    for (Square const* p = knightPtr(toCheck); (occupiedSquare = *p) != INVALID_SQUARE; ++p) {
        Piece nearPiece = pieces[occupiedSquare];
        if (isKnight(nearPiece) && isFriendly(nearPiece, enemyColor)) {
            return true;
        }
    }

    return false;
}

// Throws an error if the move is not valid.
// The gamestate is assumed to be legal (eg no pawns on final row)
struct Move GameState::validateMove(Square from, Square to) {
    if (outOfBounds(from) || outOfBounds(to)) {
        throw Error{"Invalid move: to and from squares must be between 0 and 63 inclusive"};
    }

    Piece toMove = pieces[from];
    PieceCharacteristic movingPieceType = pieceType(toMove);
    Color color = pieceColor(toMove);
    Piece targetPiece = pieces[to];

    // Check if piece can't be moved by player
    if (toMove == NO_PIECE) {
        throw Error("Invalid move: trying to move an empty square");
    } else if (toAct != color) {
        throw Error("Invalid move: trying to move enemy piece");
    }

    // Check if the destination is friendly
    // This is an error UNLESS it's a king->rook
    if (isFriendly(targetPiece, toAct) && !(movingPieceType == PIECE_KING && isRook(targetPiece))) {
        throw Error("Invalid move: trying to capture friendly piece");
    }

    // Type will be overridden in special cases: en passant, castling
    MoveType moveType;
    if (isEnemy(targetPiece, toAct)) {
        moveType = CAPTURE;
    } else {
        moveType = MOVE;
    }

    // Check pawn moves
    if (movingPieceType == PIECE_PAWN) {
        // Check for board wrap and other overly lateral moves
        int hDist = horizontalDistance(from, to);
        bool isEnPassant = to == enPassantSquare;

        int diff = from - to;
        if (toAct == WHITE ? diff < 0 : diff > 0) {
            throw Error("Invalid move: pawns cannot move backward");
        } else if (hDist == 0 && targetPiece != NO_PIECE) {
            throw Error("Invalid move: pawns cannot move forward onto other pieces");
        } else if (hDist == 1 && targetPiece == NO_PIECE && !isEnPassant) {
            throw Error("Invalid move: pawns can only move diagonally when capturing");
        } else if (hDist > 1) {
            throw Error("Invalid move: pawns can't move like that");
        }

        // Distance is 7/9 for captures, 8 for move, 16 for forced march
        // White is reversed since they move from high Squares to low ones
        int dist = diff < 0 ? -diff : diff;

        // Check forward move and captures
        switch (dist) {
            case 7: // Capture is fine
            case 9:
                if (isEnPassant) {
                    moveType = EN_PASSANT;
                } // FALL THROUGH FOR PROMOTION CHECK
            case 8: // Check if we need promotion
                if (onPromotionRow(to)) {
                    moveType = NEED_PROMOTE;
                }
                break;
            case 16: { // Forced march
                if (!onHomeRow(from, toAct)) {
                    throw Error{"Invalid move: pawns can only perform forced march from home row"};
                }

                // Safe from out-of-bounds on legal boards because pawns MUST promote
                Piece betweenHomeAndTarget = pieces[squareBehind(to, toAct)];
                if (betweenHomeAndTarget != NO_PIECE) {
                    throw Error("Invalid move: pawn cannot force march through another piece");
                }

                moveType = FORCED_MARCH;
                break;
            }
            default:
                throw Error("Invalid move: pawns can't move like that");
        }
    } else if (movingPieceType == PIECE_KNIGHT) { // Check knight moves
        if (!validKnightMovement(from, to)) {
            throw Error("Invalid move: knights can't move like that");
        }
    } else if (movingPieceType == PIECE_BISHOP) { // Check bishop moves
        // The direction of movement must be diagonal and they must not be blocked
        Direction dir = getDirection(from, to);

        if (!isDiagonal(dir)) {
            throw Error("Invalid move: bishops can't move like that");
        }

        if (isBlocked(from, to, dir)) {
            throw Error("Invalid move: bishop is blocked");
        }
    } else if (movingPieceType == PIECE_ROOK) { // Check rook moves
        Direction dir = getDirection(from, to);

        if (!isStraight(dir)) {
            throw Error("Invalid move: rooks can't move like that");
        }

        if (isBlocked(from, to, dir)) {
            throw Error("Invalid move: rook is blocked");
        }
    } else if (movingPieceType == PIECE_QUEEN) { // Check queen moves
        Direction dir = getDirection(from, to);

        if (dir == INVALID_DIRECTION) {
            throw Error("Invalid move: queens can't move like that");
        }

        if (isBlocked(from, to, dir)) {
            throw Error("Invalid move: queen is blocked");
        }
    } else if (movingPieceType == PIECE_KING) {
        bool valid = true;
        if (!inKingRange(from, to)) {
            // Check for castling, the only case when kings can move > 1 square
            if (targetPiece == NO_PIECE || (isRook(targetPiece) && isFriendly(targetPiece, color))) {
                Direction dir = getDirection(from, to);

                // For castles, store rook in 'to' so we can move it easily regardless of Chess960
                if (dir == WEST) {
                    validateCastle(true);
                    to = toAct == WHITE ? whiteRookWest : blackRookWest;
                    moveType = CASTLE_WEST;
                } else if (dir == EAST) {
                    validateCastle(false);
                    to = toAct == WHITE ? whiteRookEast : blackRookEast;
                    moveType = CASTLE_EAST;
                } else {
                    valid = false;
                }
            } else {
                valid = false;
            }
        }
        if (!valid) {
            throw Error("Invalid move: kings can't move like that");
        }
    }

    // Finally, it is illegal to put your own king in check
    // validateCastle ensures that castle moves aren't losing (isLosing doesn't handle castle cases)
    if (moveType != CASTLE_EAST && moveType != CASTLE_WEST && isLosing(from, to)) {
        throw Error("Invalid move: move would put your king in check");
    }

    return Move{from, to, moveType};
}

// Assumes otherwise valid from and to, and does NOT work in castle cases (validateCastle ensures the move isn't losing)
bool GameState::isLosing(const Square from, const Square to) {
    bool isLosing = false;
    bool isEnPassant = false;

    Piece moving = pieces[from];
    Piece target = pieces[to];

    Square epPawnSquare = INVALID_SQUARE;
    Piece epPiece = NO_PIECE;

    // Fake the move for a moment
    // To do so, we must move the piece, deal with king locations, and remove captured pieces (including en passant)
    Square kingSquare;
    if (isKing(moving)) {
        kingSquare = to;
    } else {
        kingSquare = toAct == WHITE ? whiteKingSquare : blackKingSquare;

        // The only time this can be true is during an en passant (en passant square guaranteed empty)
        if(to == enPassantSquare && isPawn(moving)) {
            isEnPassant = true;
            epPawnSquare = squareBehind(to, toAct);
            epPiece = pieces[epPawnSquare]; // The captured piece will be behind the moved piece
        }
    }

    pieces[from] = NO_PIECE;
    pieces[to] = moving;
    if(isEnPassant) {
        pieces[epPawnSquare] = NO_PIECE;
    }

    if (kingSquare != INVALID_SQUARE && isThreatenedBy(kingSquare, enemyColor(toAct))) {
        isLosing = true;
    }

    // Put the board back the way it was
    pieces[from] = moving;
    pieces[to] = target;
    if(isEnPassant) {
        pieces[epPawnSquare] = epPiece;
    }

    return isLosing;
}

// Returns true if a piece needs to be promoted
void GameState::makeMove(Move m) {
    Square from = m.from;
    Square to = m.to;
    Piece toMove = pieces[from];
    Square newEnPassantSquare = INVALID_SQUARE; // Gets cleared unless a pawn performs a forced march

    // If king moves, update king location and invalidate all castling for that color
    if (isKing(toMove)) {
        if (toAct == WHITE) {
            whiteKingSquare = to;
            whiteRookEast = INVALID_SQUARE;
            whiteRookWest = INVALID_SQUARE;
        } else {
            blackKingSquare = to;
            blackRookEast = INVALID_SQUARE;
            blackRookWest = INVALID_SQUARE;
        }
    }

    // If rook moves, invalidate its ability to castle
    if (isRook(toMove)) {
        if (toAct == WHITE) {
            if (to == whiteRookWest) {
                whiteRookWest = INVALID_SQUARE;
            } else if (to == whiteRookEast) {
                whiteRookEast = INVALID_SQUARE;
            }
        } else {
            if (to == blackRookWest) {
                blackRookWest = INVALID_SQUARE;
            } else if (to == whiteRookEast) {
                blackRookEast = INVALID_SQUARE;
            }
        }
    }

    pieces[to] = toMove;
    pieces[from] = NO_PIECE;

    // Do move specific tasks like setting newEnPassantSquare, removing en passant-ed pawns,
    // updating king squares, and invalidating castling
    switch (m.type) {
        case FORCED_MARCH:
            newEnPassantSquare = squareBehind(to, toAct);
            break;
        case EN_PASSANT: {
            Square capturedPawnSquare = squareBehind(to, toAct);
            pieces[capturedPawnSquare] = NO_PIECE;
            break;
        }
        case PROMOTION_QUEEN:
        case PROMOTION_QUEEN_CAPTURE:
            pieces[to] = toAct == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
            break;
        case PROMOTION_ROOK:
        case PROMOTION_ROOK_CAPTURE:
            pieces[to] = toAct == WHITE ? WHITE_ROOK : BLACK_ROOK;
            break;
        case PROMOTION_BISHOP:
        case PROMOTION_BISHOP_CAPTURE:
            pieces[to] = toAct == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
            break;
        case PROMOTION_KNIGHT:
        case PROMOTION_KNIGHT_CAPTURE:
            pieces[to] = toAct == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
            break;
        case CASTLE_EAST: // h-side or east
            pieces[to] = NO_PIECE;
            pieces[from] = NO_PIECE;
            if (toAct == WHITE) {
                pieces[CASTLE_EAST_WHITE_ROOK] = WHITE_ROOK;
                pieces[CASTLE_EAST_WHITE_KING] = WHITE_KING;
                whiteKingSquare = CASTLE_EAST_WHITE_KING;
            } else {
                pieces[CASTLE_EAST_BLACK_ROOK] = BLACK_ROOK;
                pieces[CASTLE_EAST_BLACK_KING] = BLACK_KING;
                blackKingSquare = CASTLE_EAST_BLACK_KING;
            }
            break;
        case CASTLE_WEST: // a-side or west
            pieces[to] = NO_PIECE;
            pieces[from] = NO_PIECE;
            if (toAct == WHITE) {
                pieces[CASTLE_WEST_WHITE_ROOK] = WHITE_ROOK;
                pieces[CASTLE_WEST_WHITE_KING] = WHITE_KING;
                whiteKingSquare = CASTLE_WEST_WHITE_KING;
            } else {
                pieces[CASTLE_WEST_BLACK_ROOK] = BLACK_ROOK;
                pieces[CASTLE_WEST_BLACK_KING] = BLACK_KING;
                blackKingSquare = CASTLE_WEST_BLACK_KING;
            }
            break;
        default:
            break;
    }

    enPassantSquare = newEnPassantSquare;
    toAct = enemyColor(toAct);
}

std::vector<Move> GameState::generateMoves() {
    std::vector<Move> moves;

    for (Square from = 0; from < 64; from++) {
        Piece fromPiece = pieces[from];

        if (pieceColor(fromPiece) != toAct) {
            continue;
        }

        switch (fromPiece & MASK_PIECE) {
            // Pawns: Not many useful precalculations, so just examine each case
            case PIECE_PAWN: {
                Square move, forcedMarch, takeW, takeE;
                if (toAct == WHITE) {
                    move = (Square) (from - 8);
                    forcedMarch = (Square) (move - 8);
                } else {
                    move = (Square) (from + 8);
                    forcedMarch = (Square) (move + 8);
                }
                takeW = (Square) (move - 1);
                takeE = (Square) (move + 1);

                if (pieces[move] == NO_PIECE) {
                    // Check losing separately because forced march might eg block a king attack
                    if (!isLosing(from, move)) {
                        if (onPromotionRow(move)) { // Add queen and knight promotions, as the rest are useless
                            moves.emplace_back(from, move, PROMOTION_QUEEN);
                            moves.emplace_back(from, move, PROMOTION_KNIGHT);
                        } else {
                            moves.emplace_back(from, move, MOVE);
                        }
                    }

                    // Since we can move forward, we check if we can also forced march (OOB check not necessary)
                    if (onHomeRow(from, toAct) && pieces[forcedMarch] == NO_PIECE && !isLosing(from, forcedMarch)) {
                        moves.emplace_back(from, forcedMarch, FORCED_MARCH);
                    }
                }

                // Check the take moves. Horizontal distance will != 1 if the move would wrap
                if (isEnemy(pieces[takeW], toAct) && horizontalDistance(from, takeW) == 1 &&
                    !isLosing(from, takeW)) {
                    if (onPromotionRow(takeW)) { // Add queen and knight promotions, as the rest are useless
                        moves.emplace_back(from, takeW, PROMOTION_QUEEN_CAPTURE);
                        moves.emplace_back(from, takeW, PROMOTION_KNIGHT_CAPTURE);
                    } else {
                        moves.emplace_back(from, takeW, CAPTURE);
                    }
                }

                if (isEnemy(pieces[takeE], toAct) && horizontalDistance(from, takeE) == 1 &&
                    !isLosing(from, takeE)) {
                    if (onPromotionRow(takeE)) { // Add queen and knight promotions, as the rest are useless
                        moves.emplace_back(from, takeE, PROMOTION_QUEEN_CAPTURE);
                        moves.emplace_back(from, takeE, PROMOTION_KNIGHT_CAPTURE);
                    } else {
                        moves.emplace_back(from, takeE, CAPTURE);
                    }
                }
                break;
            } // END PAWN CASE

            // Knights: examine squares from knight data
            case PIECE_KNIGHT: {
                Square to;
                for (Square const* p = knightPtr(from); (to = *p) != INVALID_SQUARE; ++p) {
                    Piece target = pieces[to];

                    if (!isFriendly(target, toAct) && !isLosing(from, to)) {
                        if (target == NO_PIECE) {
                            moves.emplace_back(from, to, MOVE);
                        } else {
                            moves.emplace_back(from, to, CAPTURE);
                        }
                    }
                }
                break;
            } // END KNIGHT CASE

                // Bishops: loop through direction data from NE to SW
            case PIECE_BISHOP: {
                for (int dir = DIAGONAL_START; dir <= DIAGONAL_END; ++dir) {
                    int inc = dirIncrement(dir);
                    Square to;

                    for (Square const* p = dirPtr(dir, from); (to = *p) != INVALID_SQUARE; p += inc) {
                        Piece target = pieces[to];

                        // If it's a piece, the rest are blocked
                        // Friendly pieces are not valid moves
                        if (isFriendly(target, toAct)) {
                            break;
                        } else if (!isLosing(from, to)) {
                            if (target == NO_PIECE) {
                                moves.emplace_back(from, to, MOVE);
                            } else {
                                moves.emplace_back(from, to, CAPTURE);
                                break;
                            }
                        }
                    }
                }
                break;
            } // END BISHOP CASE

            // Rooks: loop through direction data from N to W
            case PIECE_ROOK: {
                for (int dir = RANKFILE_START; dir <= RANKFILE_END; ++dir) {
                    int inc = dirIncrement(dir);
                    Square to;
                    for (Square const* p = dirPtr(dir, from); (to = *p) != INVALID_SQUARE; p += inc) {
                        Piece target = pieces[to];

                        // If it's a piece, the rest are blocked
                        // Friendly pieces are not valid moves
                        if (isFriendly(target, toAct)) {
                            break;
                        } else if (!isLosing(from, to)) {
                            if (target == NO_PIECE) {
                                moves.emplace_back(from, to, MOVE);
                            } else {
                                moves.emplace_back(from, to, CAPTURE);
                                break;
                            }
                        }
                    }
                }
                break;
            } // END ROOK CASE

                // Queens: loop through direction data from N to SW
            case PIECE_QUEEN: {
                for (int dir = ALLDIR_START; dir <= ALLDIR_END; ++dir) {
                    int inc = dirIncrement(dir);
                    Square to;
                    for (Square const* p = dirPtr(dir, from); (to = *p) != INVALID_SQUARE; p += inc) {
                        Piece target = pieces[to];

                        // If it's a piece, the rest are blocked
                        // Friendly pieces are not valid moves
                        if (isFriendly(target, toAct)) {
                            break;
                        } else if (!isLosing(from, to)) {
                            if (target == NO_PIECE) {
                                moves.emplace_back(from, to, MOVE);
                            } else {
                                moves.emplace_back(from, to, CAPTURE);
                                break;
                            }
                        }
                    }
                }
                break;
            } // END QUEEN CASE

                // Kings: examine squares from king data, then check castling
            case PIECE_KING: {
                Square to;
                for (Square const* p = kingPtr(from); (to = *p) != INVALID_SQUARE; ++p) {
                    Piece target = pieces[to];

                    if (!isFriendly(target, toAct) && !isLosing(from, to)) {
                        if (target == NO_PIECE) {
                            moves.emplace_back(from, to, MOVE);
                        } else {
                            moves.emplace_back(from, to, CAPTURE);
                        }
                    }
                }

                if(canCastle(true) == CASTLE_SUCCESS) {
                    moves.emplace_back(from, blackRookWest, CASTLE_WEST);
                }

                if(canCastle(false) == CASTLE_SUCCESS) {
                    moves.emplace_back(from, blackRookEast, CASTLE_EAST);
                }
                break;
            } // END KING CASE

            default:
                break;
        }
    }
    return moves;
}

bool GameState::currentPlayerInCheck() const {
    if (toAct == WHITE) {
        return isThreatenedBy(whiteKingSquare, BLACK);
    } else {
        return isThreatenedBy(blackKingSquare, WHITE);
    }
}



GameState::CastleResult GameState::canCastle(bool west) {
    Square kingSquare, rookSquare, targetSquare;

    if (toAct == WHITE) {
        rookSquare = west ? whiteRookWest : whiteRookEast;
        targetSquare = west ? CASTLE_WEST_WHITE_KING : CASTLE_EAST_WHITE_KING;
        kingSquare = whiteKingSquare;
    } else {
        rookSquare = west ? blackRookWest : blackRookEast;
        targetSquare = west ? CASTLE_WEST_BLACK_KING : CASTLE_EAST_BLACK_KING;
        kingSquare = blackKingSquare;
    }

    if (rookSquare == INVALID_SQUARE) {
       return CASTLE_ERR_MOVED;
    }

    // There must be no pieces between the king and the rook square
    int rookInc = signOf(rookSquare - kingSquare);
    for (auto s = (Square) (rookInc + kingSquare); s != rookSquare; s += rookInc) {
        if (pieces[s] != NO_PIECE) {
            return CASTLE_ERR_BLOCKED;
        }
    }

    // The king cannot be in check or move through a square under threat
    Color enemy = enemyColor(toAct);
    if (isThreatenedBy(kingSquare, enemy)) {
        return CASTLE_ERR_KING_CHECK;
    }

    int kingInc = signOf(targetSquare - kingSquare);
    do {
        kingSquare += kingInc;
        if (isThreatenedBy(kingSquare, enemy)) {
            return CASTLE_ERR_SQUARE_CHECK;
        }
    } while (kingSquare != targetSquare);

    return CASTLE_SUCCESS;
}

void GameState::validateCastle(bool west) {
    switch(canCastle(west)) {
        case CASTLE_SUCCESS:break;
        case CASTLE_ERR_MOVED:
            throw Error("Cannot castle: either the king or rook involved have moved");
        case CASTLE_ERR_BLOCKED:
            throw Error("Cannot castle: piece in the way");
        case CASTLE_ERR_KING_CHECK:
            throw Error("Cannot castle: king is in check");
        case CASTLE_ERR_SQUARE_CHECK:
            throw Error("Cannot castle: square between king and target square is threatened");
    }
}

GameState::GameState()
        : pieces{
        BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK,
        BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
        NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
        NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
        NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
        NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
        WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
        WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK
} {}
