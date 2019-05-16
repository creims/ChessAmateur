#include <regex>
#include <map>
#include <functional>
#include <iostream>
#include "Game.h"
#include "GameState.h"

using namespace CA3;
using std::string;
using std::vector;
using std::map;

struct GameImpl {
public:
    GameImpl();

    void newGame();

    string lastMoveString();

    string getBoard();

    void setBoard(string newBoard);

    MoveResult tryMove(Square from, Square to);

    MoveResult makeMove(Move m);

    bool canMove(Square square);

    vector<Move> getMoves();

    MoveResult promote(PromotionChoice toPromote);

    void setActivePlayer(Color c);

    Color getActivePlayer();

private:
    GameState gs{};
    vector<Move> moves;
    vector<Move> possibleMoves;
    Move incompleteMove{};

    MoveResult checkGameOver();
};

Piece charToPiece(const unsigned char c) {
    static const map<unsigned char, Piece> m{{'p', BLACK_PAWN},
                                             {'n', BLACK_KNIGHT},
                                             {'b', BLACK_BISHOP},
                                             {'r', BLACK_ROOK},
                                             {'q', BLACK_QUEEN},
                                             {'k', BLACK_KING},
                                             {'P', WHITE_PAWN},
                                             {'N', WHITE_KNIGHT},
                                             {'B', WHITE_BISHOP},
                                             {'R', WHITE_ROOK},
                                             {'Q', WHITE_QUEEN},
                                             {'K', WHITE_KING},
                                             {'.', NO_PIECE}};
    return m.at(c);
}

char pieceToChar(const Piece p) {
    static const map<Piece, unsigned char> m{{BLACK_PAWN,   'p'},
                                             {BLACK_KNIGHT, 'n'},
                                             {BLACK_BISHOP, 'b'},
                                             {BLACK_ROOK,   'r'},
                                             {BLACK_QUEEN,  'q'},
                                             {BLACK_KING,   'k'},
                                             {WHITE_PAWN,   'P'},
                                             {WHITE_KNIGHT, 'N'},
                                             {WHITE_BISHOP, 'B'},
                                             {WHITE_ROOK,   'R'},
                                             {WHITE_QUEEN,  'Q'},
                                             {WHITE_KING,   'K'},
                                             {NO_PIECE,     '.'}};
    return m.at(p);
}

constexpr char fileFromSquare(Square s) {
    return (char) ('a' + (s % 8));
}

constexpr char rankFromSquare(Square s) {
    return (char) ('0' + (8 - s / 8));
}

GameImpl::GameImpl() {
    static const string DEFAULT_BOARD_STR = "rnbqkbnr"
                                            "pppppppp"
                                            "........"
                                            "........"
                                            "........"
                                            "........"
                                            "PPPPPPPP"
                                            "RNBQKBNR";
    setBoard(DEFAULT_BOARD_STR);
}

string GameImpl::getBoard() {
    char cstr[64];

    for (int i = 0; i < 64; i++) {
        cstr[i] = pieceToChar(gs[i]);
    }

    return string(cstr, 64);
}

void GameImpl::setBoard(string newBoard) {
    using std::regex;
    using std::smatch;

    const regex multipleKings(R"((:?k.*k)|(:?K.*K))");
    const regex bothKings(R"((k).*(K))");
    const regex illegalCharacter(R"(([^rnbqpkRNBQPK\.\s]))");
    Square wk, bk;

    smatch m;
    string parsedBoard;

    // This removes whitespace
    newBoard.erase(remove_if(newBoard.begin(), newBoard.end(),
                             [](char ch) { return std::isspace<char>(ch, std::locale::classic()); }),
                   newBoard.end());

    if (newBoard.length() != 64) {
        throw Error{"Invalid board: boards must have 64 pieces (use '.' for empty squares)"};
    }

    if (regex_search(newBoard, m, multipleKings) || !regex_search(newBoard, m, bothKings)) {
        throw Error{"Invalid board: boards must have exactly one king of each color"};
    }

    bk = (Square) m.position(1);
    wk = (Square) m.position(2);

    if (regex_search(newBoard, m, illegalCharacter)) {
        throw Error{string("Invalid board: illegal character ").append(m[0])};
    }

    for (int i = 0; i < 64; i++) {
        gs[i] = charToPiece((unsigned char) newBoard[i]);
    }

    gs.setWhiteKingLocation(wk);
    gs.setBlackKingLocation(bk);

    possibleMoves = gs.generateMoves();
}

MoveResult GameImpl::tryMove(Square from, Square to) {
    Move m = gs.validateMove(from, to);
    if (m.type == NEED_PROMOTE) {
        incompleteMove = m;
        return CHOOSE_PROMOTION;
    }
    return makeMove(m);
}

MoveResult GameImpl::makeMove(Move m) {
    gs.makeMove(m);
    moves.emplace_back(m);

    return checkGameOver();
}

void GameImpl::setActivePlayer(Color c) {
    gs.setToAct(c);
}

Color GameImpl::getActivePlayer() {
    return gs.getToAct();
}

MoveResult GameImpl::promote(PromotionChoice toPromote) {
    bool capture = gs[incompleteMove.to] != NO_PIECE;
    MoveType type;
    switch (toPromote) {
        case ROOK:
            type = capture ? PROMOTION_ROOK_CAPTURE : PROMOTION_ROOK;
            break;
        case BISHOP:
            type = capture ? PROMOTION_BISHOP_CAPTURE : PROMOTION_BISHOP;
            break;
        case KNIGHT:
            type = capture ? PROMOTION_KNIGHT_CAPTURE : PROMOTION_KNIGHT;
            break;
        case QUEEN:
        default:
            type = capture ? PROMOTION_QUEEN_CAPTURE : PROMOTION_QUEEN;
            break;
    }

    incompleteMove.type = type;
    return makeMove(incompleteMove);
}

MoveResult GameImpl::checkGameOver() {
    MoveResult result = GAME_CONTINUES;
    possibleMoves = gs.generateMoves();
    if (possibleMoves.empty()) {
        if (gs.currentPlayerInCheck()) {
            result = gs.getToAct() == WHITE ? BLACK_WINS : WHITE_WINS;
        } else {
            result = STALEMATE;
        }
    }

    return result;
}

vector<Move> GameImpl::getMoves() {
    return possibleMoves;
}

bool GameImpl::canMove(Square square) {
    return isFriendly(gs[square], gs.getToAct());
}

void GameImpl::newGame() {
    moves.clear();
    possibleMoves.clear();
    gs = GameState{};
}

string GameImpl::lastMoveString() {
    if (moves.empty()) {
        throw Error{"lastMoveString error: no moves"};
    }

    string ret{};
    Move m = moves.back();

    // Check for castling
    if(m.type == CASTLE_EAST) {
        return "O-O";
    } else if(m.type == CASTLE_WEST) {
        return "O-O-O";
    }

    Square from = m.from;
    Square to = m.to; // to contains the new piece except for castling
    Piece pieceMoved = gs[to];
    string pieceStr; // Stores the piece in string form OR, for promotions, the piece promoted to
    bool isPromotion = m.isPromotion();
    PieceCharacteristic pcType = pieceType(pieceMoved);

    switch (pcType) {
        case PIECE_KNIGHT:
            pieceStr = "N";
            break;
        case PIECE_BISHOP:
            pieceStr = "B";
            break;
        case PIECE_ROOK:
            pieceStr = "R";
            break;
        case PIECE_QUEEN:
            pieceStr = "Q";
            break;
        case PIECE_KING:
            pieceStr = "K";
            break;
        default:
            break;
    }

    // If the piece has changed, it was a pawn, so don't specify it
    if(!isPromotion) {
        ret += pieceStr;
    } else { // If it was a promotion, update pcType so we can check for ambiguation
        pcType = PIECE_PAWN;
    }

    // Kings never need disambiguation, and pawns do it differently (rank only on every capture/promotion)
    if(pcType != PIECE_KING && pcType != PIECE_PAWN) {
        // Disambiguate identical pieces from the same rank, then the same file (e6 instead of 6e)
        Square toCheck;
        int inc;

        // To find pieces on the same rank, go from left to right
        inc = dirIncrement(EAST);
        for (Square const* p = rankPtr(EAST, from); (toCheck = *p) != INVALID_SQUARE; p += inc) {
            // Disambiguation is needed iff an identical piece can attack the square in question
            if (toCheck != from && gs[toCheck] == pieceMoved && gs.isThreatenedBySquare(to, toCheck)) {
                ret += fileFromSquare(from);
                break;
            }
        }
        // To find pieces on same file, go from top to bottom
        inc = dirIncrement(SOUTH);
        for (Square const* p = filePtr(SOUTH, from); (toCheck = *p) != INVALID_SQUARE; p += inc) {
            // Disambiguation is needed iff an identical piece can attack the square in question
            if (toCheck != from && gs[toCheck] == pieceMoved && gs.isThreatenedBySquare(to, toCheck)) {
                ret += rankFromSquare(from);
                break;
            }
        }
    } // End disambiguation

    // Takes
    if(m.isCapture()) {
        // Pawns identified by rank on capture/promotion
        if(pcType == PIECE_PAWN || isPromotion) {
            ret += fileFromSquare(from);
        }

        ret += 'x';
    }

    // Destination square
    ret += fileFromSquare(to);
    ret += rankFromSquare(to);

    // Add promotion at end
    if(isPromotion) {
        ret += "=" + pieceStr;
    }

    // Check for checkmate/stalemate/check
    MoveResult result = checkGameOver();
    if(result == WHITE_WINS) {
        ret += "# 1-0";
    } else if(result == BLACK_WINS) {
        ret += "# 0-1";
    } else if(result == STALEMATE) {
        ret += " ½–½";
    } else if(gs.currentPlayerInCheck()) {
        ret += "+";
    }

    return ret;
}

// Forward to implementation
Game::Game() : pimpl{std::make_unique<GameImpl>()} {}

string Game::getBoard() { return pimpl->getBoard(); }

void Game::setBoard(string newBoard) { pimpl->setBoard(move(newBoard)); }

MoveResult Game::tryMove(Square from, Square to) { return pimpl->tryMove(from, to); }

MoveResult Game::makeMove(Move m) { return pimpl->makeMove(m); }

void Game::setActivePlayer(Color c) { pimpl->setActivePlayer(c); }

Color Game::getActivePlayer() { return pimpl->getActivePlayer(); }

MoveResult Game::promote(PromotionChoice toPromote) { return pimpl->promote(toPromote); }

vector<Move> Game::getMoves() { return pimpl->getMoves(); }

bool Game::canMove(Square square) { return pimpl->canMove(square); }

void Game::newGame() { pimpl->newGame(); }

std::string Game::lastMoveString() { return pimpl->lastMoveString(); }

Game::~Game() = default;
