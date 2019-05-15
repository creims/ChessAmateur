const whiteColor = '#EEEED2';
const whiteHoverColor = '#F5A4B0';
const whiteMoveColor = '#F6F681';
const blackColor = '#769656';
const blackHoverColor = '#B76E79';
const blackMoveColor = '#BBCA44';
const Pieces = {'P': 0, 'N': 1, 'B': 2, 'R': 3, 'Q': 4, 'K': 5, 'p': 6, 'n': 7, 'b': 8, 'r': 9, 'q': 10, 'k': 11};
const filenames = ['img/wpawn.svg', 'img/wknight.svg', 'img/wbishop.svg',
    'img/wrook.svg', 'img/wqueen.svg', 'img/wking.svg',
    'img/bpawn.svg', 'img/bknight.svg', 'img/bbishop.svg',
    'img/brook.svg', 'img/bqueen.svg', 'img/bking.svg'];

let svgs = [];
let svgsLoaded = false;
let boardsToDraw = [];

function loadSVGs() {
    let loadedCount = 0;
    for (let file of filenames) {
        let img = new Image();
        // After all images are loaded, draw the board
        img.onload = () => {
            loadedCount++;
            if (loadedCount >= filenames.length) {
                svgsLoaded = true;
                for (let board of boardsToDraw) {
                    board.drawAll();
                }
            }
        };
        img.src = file;
        svgs.push(img);
    }
}

// A square is white if its number is even for odd rows, reversed for even rows
function isWhite(square) {
    return !!((square % 2 === 0) ^ (square % 16 > 7));
}

function getNormalColor(square) {
    return isWhite(square) ? whiteColor : blackColor;
}

function getHoverColor(square) {
    return isWhite(square) ? whiteHoverColor : blackHoverColor;
}

function getMoveColor(square) {
    return isWhite(square) ? whiteMoveColor : blackMoveColor;
}

function clamp(num, min, max) {
    return Math.min(Math.max(num, min), max);
}

// Takes a node and returns a copy that prevents default drag behavior
function makeUndraggableCopy(img) {
    const copy = img.cloneNode();
    copy.ondragstart = e => e.preventDefault();
    return copy;
}

function Chessboard(parentElement, engine) {
    this.parentElement = parentElement;
    this.engineAPI = engine;

    this.boardCanvas = document.createElement('canvas');
    this.piecesCanvas = document.createElement('canvas');
    this.drawCanvas = document.createElement('canvas');
    this.boardInterface = document.createElement('div');

    this.boardCanvas.style.zIndex = '-4';
    this.piecesCanvas.style.zIndex = '-3';
    this.drawCanvas.style.zIndex = '-2';

    parentElement.appendChild(this.boardCanvas);
    parentElement.appendChild(this.piecesCanvas);
    parentElement.appendChild(this.drawCanvas);
    parentElement.appendChild(this.boardInterface);

    this.boardCtx = this.boardCanvas.getContext('2d');
    this.piecesCtx = this.piecesCanvas.getContext('2d');
    this.drawCtx = this.drawCanvas.getContext('2d');

    this.dialogs = {};
    this.dialogs.whitePromo = this.makePromoDialog(true);
    this.dialogs.blackPromo = this.makePromoDialog(false);
    this.dialogs.whiteVictory = this.makeVictoryDialog(true);
    this.dialogs.blackVictory = this.makeVictoryDialog(false);
    this.dialogs.stalemate = this.makeStalemateDialog();

    this.parentElement.appendChild(this.dialogs.whitePromo);
    this.parentElement.appendChild(this.dialogs.blackPromo);
    this.parentElement.appendChild(this.dialogs.whiteVictory);
    this.parentElement.appendChild(this.dialogs.blackVictory);
    this.parentElement.appendChild(this.dialogs.stalemate);

    this.boardInterface.oncontextmenu = (e) => {
        e.preventDefault();
    };

    let handleDown = (e) => {
        if (!this.holdingPiece) {
            if (e.type === 'touchstart') e = e.targetTouches[0];
            this.updateUserCoords(e.clientX, e.clientY);
            this.pickUpPiece();
        }
    };

    let handleUp = (e) => {
        if (this.holdingPiece) {
            if (e.type === 'touchend') e = e.targetTouches[0];
            if (document.elementFromPoint(e.clientX, e.clientY) !== this.boardInterface) {
                this.dropPiece();
            } else {
                this.updateUserCoords(e.clientX, e.clientY);
                this.movePiece();
            }
        }
    };

    let handleMove = (e) => {
        if (this.holdingPiece) {
            if (e.type === 'touchmove') e = e.targetTouches[0];
            this.updateUserCoords(e.clientX, e.clientY);
            this.drawOverlay();
        }
    };

    let handleLeave = () => {
        if (this.holdingPiece) {
            this.dropPiece();
        }
    };

    this.boardInterface.addEventListener('mousedown', handleDown);
    this.boardInterface.addEventListener('mouseup', handleUp);
    this.boardInterface.addEventListener('mousemove', handleMove);
    this.boardInterface.addEventListener('mouseleave', handleLeave);

    this.boardInterface.addEventListener('touchstart', handleDown);
    this.boardInterface.addEventListener('touchend', handleUp);
    this.boardInterface.addEventListener('touchmove', handleMove);

    // This is the "per game" state
    this.holdingPiece = false;
    this.pieceString = this.engineAPI.getPieces();
    this.resize();
}

Chessboard.prototype.reverseIfNeeded = function (square) {
    return this.reverseBoard ? 63 - square : square;
};

Chessboard.prototype.updateUserCoords = function (x, y) {
    x = clamp(x, this.bounds.left, this.bounds.right - 1);
    y = clamp(y, this.bounds.top, this.bounds.bottom - 1);
    this.userX = x - this.bounds.left;
    this.userY = y - this.bounds.top;
};

Chessboard.prototype.getSquareXCoord = function (square) {
    return (square % 8) * this.squareSize;
};

Chessboard.prototype.getSquareYCoord = function (square) {
    return Math.floor(square / 8) * this.squareSize;
};

Chessboard.prototype.squareFromCoords = function (x, y) {
    return Math.floor(x / this.squareSize) + Math.floor(y / this.squareSize) * 8;
};

Chessboard.prototype.resize = function () {
    const oldBoardSize = this.boardSize;
    this.boardSize = Math.min(this.parentElement.clientWidth, this.parentElement.clientHeight);
    this.boardSize = this.boardSize - (this.boardSize % 8); // Make sure it's divisible by 8
    this.squareSize = this.boardSize / 8;
    this.halfSquare = Math.floor(this.squareSize / 2);

    this.boardCanvas.width = this.boardCanvas.height = this.boardSize;
    this.piecesCanvas.width = this.piecesCanvas.height = this.boardSize;
    this.drawCanvas.width = this.drawCanvas.height = this.boardSize;

    this.boardInterface.style.width = this.boardInterface.style.height = this.boardSize + 'px';

    this.bounds = this.drawCanvas.getBoundingClientRect();

    const ratio = oldBoardSize ? this.boardSize / oldBoardSize : 1;
    this.dialogs.whitePromo.reposition(ratio);
    this.dialogs.blackPromo.reposition(ratio);

    if (svgsLoaded) {
        this.drawAll();
    } else {
        boardsToDraw.push(this);
    }
};

Chessboard.prototype.drawAll = function () {
    this.drawBoard();
    this.drawAllPieces();
    this.drawOverlay();
};

Chessboard.prototype.colorBoardSquare = function (square, color) {
    let startX = this.getSquareXCoord(square);
    let startY = this.getSquareYCoord(square);

    this.boardCtx.fillStyle = color;
    this.boardCtx.fillRect(startX, startY, this.squareSize, this.squareSize);
};

Chessboard.prototype.drawBoard = function () {
    let color;
    for (let square = 0; square < 64; square++) {
        this.colorBoardSquare(square, getNormalColor(square));
    }
};

Chessboard.prototype.clearSquare = function (context, square) {
    let startX = this.getSquareXCoord(square);
    let startY = this.getSquareYCoord(square);

    context.clearRect(startX, startY, this.squareSize, this.squareSize);
};

Chessboard.prototype.drawPiece = function (square, img = undefined) {
    if(img === undefined) {
       const pieceChar = this.pieceString[square];
       if (pieceChar !== '.') {
           img = svgs[Pieces[pieceChar]];
       } else {
            return;
       }
    }

    let startX = this.getSquareXCoord(square);
    let startY = this.getSquareYCoord(square);
    this.piecesCtx.drawImage(img, startX, startY, this.squareSize, this.squareSize);
};

Chessboard.prototype.drawAllPieces = function () {
    for (let square = 0; square < 64; square++) {
        // Draw the square unless it's held
        if (!(this.holdingPiece && this.heldPieceSquare === square)) {
            this.drawPiece(square);
        }
    }
};

Chessboard.prototype.updatePieces = function (forceAll = false) {
    let oldString = this.pieceString;
    this.pieceString = this.engineAPI.getPieces();
    if (this.reverseBoard) {
        this.pieceString = this.pieceString.split('').reverse().join('');
    }

    for (let square = 0; square < 64; square++) {
        let piece = this.pieceString[square];
        if (forceAll || oldString[square] !== piece) {
            this.clearSquare(this.piecesCtx, square);
            if (Pieces[piece] !== undefined) {
                this.drawPiece(square);
            }
        }
    }
};

Chessboard.prototype.pickUpPiece = function () {
    let square = this.squareFromCoords(this.userX, this.userY);
    if (!this.holdingPiece && this.engineAPI.canMove(this.reverseIfNeeded(square))) {
        this.holdingPiece = true;
        this.heldPieceSquare = square;
        this.heldPieceSVG = svgs[Pieces[this.pieceString[square]]];

        this.boardInterface.style.cursor = "none";
        this.clearSquare(this.piecesCtx, square);
        this.drawOverlay();
    }
};

Chessboard.prototype.dropPiece = function (redraw = true) {
    this.holdingPiece = false;
    this.updateHoverHighlighting();
    this.boardInterface.style.cursor = "default";
    this.drawCtx.clearRect(0, 0, this.boardSize, this.boardSize);

    if (redraw) {
        this.drawPiece(this.heldPieceSquare);
    }
};

Chessboard.prototype.setLastMoveHighlights = function(from, to) {
    if(this.lastMovedFrom !== undefined) {
        this.clearSquare(this.boardCtx, this.lastMovedFrom);
    }

    if(this.lastMovedTo !== undefined) {
        this.clearSquare(this.boardCtx, this.lastMovedTo);
    }

    this.colorBoardSquare(from, getMoveColor(from));
    this.colorBoardSquare(to, getMoveColor(to));

    this.lastMovedFrom = from;
    this.lastMovedTo = to;
};

Chessboard.prototype.updateHoverHighlighting = function(newHoverSquare = undefined) {
    if(newHoverSquare !== this.hoverSquare) {
        let color;
        if(this.hoverSquare === this.lastMovedFrom || this.hoverSquare === this.lastMovedTo) {
            color = getMoveColor(this.hoverSquare);
        } else {
            color = getNormalColor(this.hoverSquare);
        }
        this.colorBoardSquare(this.hoverSquare, color);

        if(newHoverSquare !== undefined) {
            this.colorBoardSquare(newHoverSquare, getHoverColor(newHoverSquare));
        }
    }

    this.hoverSquare = newHoverSquare;
};

Chessboard.prototype.movePiece = function () {
    if (this.holdingPiece) {
        let targetSquare = this.squareFromCoords(this.userX, this.userY);
        let putBack = true;

        if (this.heldPieceSquare !== targetSquare &&
            this.engineAPI.tryMove(this.reverseIfNeeded(this.heldPieceSquare), this.reverseIfNeeded(targetSquare))) {
            this.updateHoverHighlighting();
            this.setLastMoveHighlights(this.heldPieceSquare, targetSquare);
            this.updatePieces();
            putBack = false;
        }

        this.dropPiece(putBack);
    }
};

Chessboard.prototype.drawOverlay = function () {
    if (this.holdingPiece) {
        this.drawCtx.clearRect(0, 0, this.boardSize, this.boardSize);
        this.drawCtx.drawImage(this.heldPieceSVG, this.userX - this.halfSquare, this.userY - this.halfSquare,
            this.squareSize, this.squareSize);

        this.updateHoverHighlighting(this.squareFromCoords(this.userX, this.userY));
    }
};

Chessboard.prototype.makePromoDialog = function (isWhite) {
    const pcs = isWhite ? ['Q', 'R', 'B', 'N'] : ['q', 'r', 'b', 'n'];
    const choices = [this.engineAPI.PromotionChoices.QUEEN, this.engineAPI.PromotionChoices.ROOK,
        this.engineAPI.PromotionChoices.BISHOP, this.engineAPI.PromotionChoices.KNIGHT];
    let promoDialog = document.createElement('div');
    promoDialog.className = 'chess-dialog promotion-dialog';

    let makePromoBtn = (num, svg) => {
        let btn = makeUndraggableCopy(svg);
        btn.className = 'piece-btn';
        btn.onclick = () => {
            promoDialog.style.visibility = 'hidden';
            this.engineAPI.promote(num);
            this.updatePieces();
            this.enableMoves();
        };

        return btn;
    };

    for (let i = 0; i < 4; i++) {
        const svg = svgs[Pieces[pcs[i]]];
        promoDialog.appendChild(makePromoBtn(choices[i], svg));
    }

    promoDialog.reposition = (ratio = 1) => {
        promoDialog.top = Math.floor(promoDialog.top * ratio);
        promoDialog.left = Math.floor(promoDialog.left * ratio);
        promoDialog.style.top = promoDialog.top + 'px';
        promoDialog.style.left = promoDialog.left + 'px';
    };

    return promoDialog;
};

Chessboard.prototype.makeVictoryDialog = function (isWhite) {
    const victoryDialog = document.createElement('div');
    const victoryText = document.createElement('span');
    const icon = makeUndraggableCopy(svgs[Pieces[isWhite ? 'K' : 'k']]);
    victoryText.innerText = isWhite ? "White Victory!" : "Black Victory!";

    victoryDialog.appendChild(victoryText);
    victoryDialog.appendChild(icon);

    victoryDialog.className = "chess-dialog gameover-dialog victory-dialog";

    return victoryDialog;
};

Chessboard.prototype.makeStalemateDialog = function () {
    const stalemateDialog = document.createElement('div');
    const stalemateText = document.createElement('span');
    stalemateText.innerText = "Stalemate...";

    stalemateDialog.appendChild(stalemateText);
    stalemateDialog.appendChild(makeUndraggableCopy(svgs[Pieces['K']]));
    stalemateDialog.appendChild(makeUndraggableCopy(svgs[Pieces['k']]));

    stalemateDialog.className = "chess-dialog gameover-dialog stalemate-dialog";

    return stalemateDialog;
};

Chessboard.prototype.disableMoves = function () {
    this.boardInterface.style.visibility = 'hidden';
};

Chessboard.prototype.enableMoves = function () {
    this.boardInterface.style.visibility = 'visible';
};

Chessboard.prototype.promptForPromotion = function () {
    this.disableMoves();

    // Draw the pawn on the correct square
    const promoSquare = this.squareFromCoords(this.userX, this.userY);
    this.clearSquare(this.piecesCtx, promoSquare);
    this.drawPiece(promoSquare, this.heldPieceSVG);

    let promoDialog = this.engineAPI.whiteToMove() ? this.dialogs.whitePromo : this.dialogs.blackPromo;

    promoDialog.top = this.userY < this.boardSize / 2 ? this.userY : this.userY - promoDialog.offsetHeight;
    promoDialog.left = this.userX < this.boardSize / 2 ? this.userX : this.userX - promoDialog.offsetWidth;
    promoDialog.reposition();

    promoDialog.style.visibility = 'visible';

};

Chessboard.prototype.showVictoryDialog = function () {
    this.disableMoves();

    let victoryDialog = this.engineAPI.whiteToMove() ? this.dialogs.blackVictory : this.dialogs.whiteVictory;
    victoryDialog.style.visibility = 'visible';
};

Chessboard.prototype.showStalemateDialog = function () {
    this.disableMoves();
    this.dialogs.stalemate.style.visibility = 'visible';
};

Chessboard.prototype.clearDialogs = function () {
    Object.keys(this.dialogs).forEach(key => this.dialogs[key].style.visibility = 'hidden');
    this.enableMoves();
};

Chessboard.prototype.newGame = function (reverseBoard = false) {
    this.reverseBoard = reverseBoard;

    this.holdingPiece = false;
    this.engineAPI.newGame();
    this.clearDialogs();
    this.updatePieces(true);
};

loadSVGs();

export default Chessboard;