#ifndef CHESSAMATEUR3_GAME_H
#define CHESSAMATEUR3_GAME_H

#include <string>
#include "Error.h"
#include "piece.h"
#include "Move.h"

enum MoveResult { GAME_CONTINUES = 0, WHITE_WINS = 1, BLACK_WINS = 2, STALEMATE = 3, CHOOSE_PROMOTION = 4 };
enum PromotionChoice { QUEEN = 0, ROOK = 1, BISHOP = 2, KNIGHT = 3};

class GameImpl;
class Game {
public:
    Game();

    void newGame();

    std::string lastMoveString();

    std::string getBoard();
    void setBoard(std::string newBoard);

    MoveResult tryMove(CA3::Square from, CA3::Square to);
    MoveResult makeMove(Move m);

    bool canMove(CA3::Square square);

    std::vector<Move> getMoves();

    MoveResult promote(PromotionChoice choice);

    void setActivePlayer(CA3::Color c);
    CA3::Color getActivePlayer();

    ~Game();
private:
    std::unique_ptr<GameImpl> pimpl;
};

#endif //CHESSAMATEUR3_GAME_H
