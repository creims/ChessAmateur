#ifndef CHESSAMATEUR3_GAMESTATE_H
#define CHESSAMATEUR3_GAMESTATE_H

#include "piece.h"
#include "Error.h"
#include "Move.h"
#include <vector>
#include "logistics.h"

class GameState {
public:
    GameState();

    CA3::Piece operator [] (CA3::Square i) const { return pieces[i]; }
    CA3::Piece& operator [] (CA3::Square i) { return pieces[i]; }

    void makeEmpty();

    bool isThreatenedBy(CA3::Square toCheck, CA3::Color enemyColor) const;
    bool isThreatenedBySquare(CA3::Square victim, CA3::Square attackingSquare) const;
    bool currentPlayerInCheck() const;

    // Given a from and to square describing a move, returns corresponding Move object.
    // If the move is valid, the returned Move will be ready to pass to makeMove UNLESS
    // it involves a pawn promotion, in which case the Move's type will be NEED_PROMOTION.
    // If the move is not valid, an Error will be thrown that contains a descriptive error message.
    Move validateMove(CA3::Square from, CA3::Square to);

    // Generate all valid moves in the GameState. Promotion moves will include queen and knight options
    // Will be empty if the game is over (stalemate or checkmate)
    std::vector<Move> generateMoves();

    // Should only be called with valid moves! Check them with validateMove
    // or generate them with generatePossibleMoves.
    // Returns true if the GameState needs to promote a pawn
    void makeMove(Move m);

    CA3::Color getToAct() const { return toAct; };

    // Mostly for testing
    void setToAct(CA3::Color _toAct) { toAct = _toAct; };
    void setWhiteKingLocation(CA3::Square location) { whiteKingSquare = location; };
    void setWhiteRookEastLocation(CA3::Square location) { whiteRookEast = location; };
    void setWhiteRookWestLocation(CA3::Square location) { whiteRookWest = location; };
    void setBlackKingLocation(CA3::Square location) { blackKingSquare = location; };
    void setBlackRookEastLocation(CA3::Square location) { blackRookEast = location; };
    void setBlackRookWestLocation(CA3::Square location) { blackRookWest = location; };

private:
    CA3::Piece pieces[64];
    CA3::Color toAct{CA3::WHITE};
    CA3::Square blackKingSquare{4}, whiteKingSquare{60}, enPassantSquare{CA3::INVALID_SQUARE};
    CA3::Square blackRookEast{7}, blackRookWest{0}, whiteRookEast{63}, whiteRookWest{56};

    CA3::Square nearestOccupiedInDir(CA3::Square from, CA3::Direction dir) const;
    bool isBlocked(CA3::Square from, CA3::Square to, CA3::Direction dir) const;
    bool isLosing(CA3::Square from, CA3::Square to);

    enum CastleResult : uint8_t;
    CastleResult canCastle(bool west);
    void validateCastle(bool west);
};

#endif //CHESSAMATEUR3_GAMESTATE_H
