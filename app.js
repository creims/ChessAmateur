import ChessAmateur from './module.js';
import Chessboard from './chessboard.js';

ChessAmateur.onRuntimeInitialized = () => {
    const logger = new ChessLogger(document.getElementById('moves'));

    let engine = {
        newGame: ChessAmateur.newGame,
        getPieces: ChessAmateur.getPieces,
        tryMove: ChessAmateur.tryMove,
        canMove: ChessAmateur.canMove,
        promote: ChessAmateur.promote,
        whiteToMove: ChessAmateur.whiteToMove,
        PromotionChoices: ChessAmateur.PromotionChoices,
    };

    const board = new Chessboard(document.getElementById('board'), engine);

    ChessAmateur.registerErrorHandler((msg) => {
        console.log(msg);
    });

    ChessAmateur.registerLogHandler((moveString) => {
        logger.logMove(moveString);
    });

    ChessAmateur.registerPromotionHandler(() => {
        board.promptForPromotion();
    });

    ChessAmateur.registerVictoryHandler(() => {
        board.showVictoryDialog();
    });

    ChessAmateur.registerStalemateHandler(() => {
        board.showStalemateDialog();
    });

    window.addEventListener('resize', () => {
        board.resize();
        logger.scrollToBottom();
    });

    document.getElementById('newgame-white').onclick = () => {
        logger.clear();
        board.newGame(false);
    };

    document.getElementById('newgame-black').onclick = () => {
        logger.clear();
        board.newGame(true);
    };
};

function ChessLogger(moveLog) {
    this.currentSpan = undefined;
    this.moveCount = 0;
    this.whiteMove = true;
    this.moveLog = moveLog;

    this.logMove = (moveString) => {
        if (this.whiteMove) {
            this.moveCount++;
            this.currentSpan = document.createElement('span');
            this.currentSpan.innerHTML = this.moveCount + '.&emsp;' + moveString;
            this.moveLog.appendChild(this.currentSpan);
            this.scrollToBottom();
        } else {
            this.currentSpan.innerHTML += '&emsp;' + moveString;
        }
        this.whiteMove = !this.whiteMove;
    };

    this.clear = () => {
        this.whiteMove = true;
        this.moveCount = 0;
        this.moveLog.innerHTML = "";
    };

    this.scrollToBottom = () => { this.moveLog.scrollTop = this.moveLog.scrollHeight; };
}
