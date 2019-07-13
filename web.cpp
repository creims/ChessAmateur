#include <emscripten/bind.h>
#include <iostream>
#include <string>
#include "src/Game.h"
#include "src/Error.h"

emscripten::val errorHandler = emscripten::val::global("console.log");
emscripten::val logHandler = emscripten::val::undefined();
emscripten::val promotionHandler = emscripten::val::undefined();
emscripten::val victoryHandler = emscripten::val::undefined();
emscripten::val stalemateHandler = emscripten::val::undefined();

Game g{};

void newGame() {
    g.newGame();
}

bool tryMove(int from, int to) {
    bool logMove = true;
    try {
        MoveResult result = g.tryMove(from, to);
        switch (result) {
            case WHITE_WINS:
            case BLACK_WINS:
                victoryHandler();
                break;
            case STALEMATE:
                stalemateHandler();
                break;
            case CHOOSE_PROMOTION:
                promotionHandler();
                logMove = false;
            default: // Nothing needs to be done
                break;
        }
    } catch (Error& e) {
        errorHandler(e.what());
        return false;
    }

    if(logMove) {
        logHandler(g.lastMoveString());
    }

    return true;
}

std::string getPieces() {
    return g.getBoard();
}

bool canMove(int square) {
    return g.canMove(square);
}

void promoteTo(PromotionChoice choice) {
    g.promote(choice);
    logHandler(g.lastMoveString());
}

bool whiteToMove() {
    return g.getActivePlayer() == CA3::WHITE;
}

void registerErrorHandler(emscripten::val cb) {
    errorHandler = cb;
}

void registerLogHandler(emscripten::val cb) {
    logHandler = cb;
}

void registerPromotionHandler(emscripten::val cb) {
    promotionHandler = cb;
}

void registerVictoryHandler(emscripten::val cb) {
    victoryHandler = cb;
}

void registerStalemateHandler(emscripten::val cb) {
    stalemateHandler = cb;
}

EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("newGame", &newGame);
        emscripten::function("getPieces", &getPieces);
        emscripten::function("canMove", &canMove);
        emscripten::function("tryMove", &tryMove);
        emscripten::function("promote", &promoteTo);
        emscripten::function("whiteToMove", &whiteToMove);

        emscripten::function("registerErrorHandler", &registerErrorHandler);
        emscripten::function("registerLogHandler", &registerLogHandler);
        emscripten::function("registerPromotionHandler", &registerPromotionHandler);
        emscripten::function("registerVictoryHandler", &registerVictoryHandler);
        emscripten::function("registerStalemateHandler", &registerStalemateHandler);

        emscripten::enum_<PromotionChoice>("PromotionChoices")
        .value("QUEEN", QUEEN)
        .value("ROOK", ROOK)
        .value("BISHOP", BISHOP)
        .value("KNIGHT", KNIGHT);
}
