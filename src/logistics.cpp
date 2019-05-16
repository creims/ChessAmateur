#include "logistics.h"
// Includes pre-generated arrays to speed up move generation, check checks, and validation
// This code seeks to minimize overhead because AI move generation spends most of its time generating moves
// and checking for check/checkmate - over 90% according to profiling

// Works by indexing into an array representing rows, columns, or diagonals
// Since 2 directions use the same rows, columns, and diagonals, use dirIncrement(Dir)
// to obtain the correct increment

// For example, to check for the nearest piece to the west from square S, iterate through e_w_squares:

// int inc = dirIncrement(WEST);
// for(i = w_indices[S]; ; i += inc) {
//   Square checked = e_w_squares[i];
//   if(checked == INVALID_SQUARE) { ... } // there is no piece west of S
//   Piece p = gamestate[checked];
//   if(p != NO_PIECE { ... } // checked is the nearest occupied square, and the piece on it is p
// }

// OR, using dirPtr to find the squares west of 34:

// Direction dir = WEST;
// Square S = 34, inDir;
// int inc = dirIncrement(dir);
// for(Square const* p = dirPtr(dir, s); (inDir = *p) != INVALID_SQUARE; p += inc) { ... } // inDir is the square

namespace CA3 {
    // DIRECTION RAY DATA
    const int s_indices[64]{
            2, 11, 20, 29, 38, 47, 56, 65, 3, 12, 21, 30, 39, 48, 57, 66, 4, 13, 22, 31, 40, 49, 58, 67, 5, 14, 23, 32,
            41, 50, 59, 68, 6, 15, 24, 33, 42, 51, 60, 69, 7, 16, 25, 34, 43, 52, 61, 70, 8, 17, 26, 35, 44, 53, 62, 71,
            9, 18, 27, 36, 45, 54, 63, 72
    };
    const int n_indices[64]{
            0, 9, 18, 27, 36, 45, 54, 63, 1, 10, 19, 28, 37, 46, 55, 64, 2, 11, 20, 29, 38, 47, 56, 65, 3, 12, 21, 30,
            39, 48, 57, 66, 4, 13, 22, 31, 40, 49, 58, 67, 5, 14, 23, 32, 41, 50, 59, 68, 6, 15, 24, 33, 42, 51, 60, 69,
            7, 16, 25, 34, 43, 52, 61, 70
    };
    const Square s_n_squares[73]{
            INVALID_SQUARE, 0, 8, 16, 24, 32, 40, 48, 56, INVALID_SQUARE, 1, 9, 17, 25, 33, 41, 49, 57, INVALID_SQUARE,
            2, 10, 18, 26, 34, 42, 50, 58, INVALID_SQUARE, 3, 11, 19, 27, 35, 43, 51, 59, INVALID_SQUARE, 4, 12, 20, 28,
            36, 44, 52, 60, INVALID_SQUARE, 5, 13, 21, 29, 37, 45, 53, 61, INVALID_SQUARE, 6, 14, 22, 30, 38, 46, 54,
            62, INVALID_SQUARE, 7, 15, 23, 31, 39, 47, 55, 63, INVALID_SQUARE
    };

    const int e_indices[64]{
            2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33,
            34, 35, 36, 38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 62, 63,
            65, 66, 67, 68, 69, 70, 71, 72
    };
    const int w_indices[64]{
            0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31,
            32, 33, 34, 36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61,
            63, 64, 65, 66, 67, 68, 69, 70
    };
    const Square e_w_squares[73]{
            INVALID_SQUARE, 0, 1, 2, 3, 4, 5, 6, 7, INVALID_SQUARE, 8, 9, 10, 11, 12, 13, 14, 15, INVALID_SQUARE, 16,
            17, 18, 19, 20, 21, 22, 23, INVALID_SQUARE, 24, 25, 26, 27, 28, 29, 30, 31, INVALID_SQUARE, 32, 33, 34, 35,
            36, 37, 38, 39, INVALID_SQUARE, 40, 41, 42, 43, 44, 45, 46, 47, INVALID_SQUARE, 48, 49, 50, 51, 52, 53, 54,
            55, INVALID_SQUARE, 56, 57, 58, 59, 60, 61, 62, 63, INVALID_SQUARE
    };

    const int se_indices[64]{
            2, 11, 19, 26, 32, 37, 41, 0, 46, 3, 12, 20, 27, 33, 38, 0, 54, 47, 4, 13, 21, 28, 34, 0, 61, 55, 48, 5, 14,
            22, 29, 0, 67, 62, 56, 49, 6, 15, 23, 0, 72, 68, 63, 57, 50, 7, 16, 0, 76, 73, 69, 64, 58, 51, 8, 0, 0, 0,
            0, 0, 0, 0, 0, 0
    };
    const int nw_indices[64]{
            0, 9, 17, 24, 30, 35, 39, 77, 0, 1, 10, 18, 25, 31, 36, 40, 52, 45, 2, 11, 19, 26, 32, 37, 59, 53, 46, 3,
            12, 20, 27, 33, 65, 60, 54, 47, 4, 13, 21, 28, 70, 66, 61, 55, 48, 5, 14, 22, 74, 71, 67, 62, 56, 49, 6, 15,
            77, 75, 72, 68, 63, 57, 50, 7
    };
    const Square se_nw_squares[80]{
            INVALID_SQUARE, 0, 9, 18, 27, 36, 45, 54, 63, INVALID_SQUARE, 1, 10, 19, 28, 37, 46, 55, INVALID_SQUARE, 2,
            11, 20, 29, 38, 47, INVALID_SQUARE, 3, 12, 21, 30, 39, INVALID_SQUARE, 4, 13, 22, 31, INVALID_SQUARE, 5, 14,
            23, INVALID_SQUARE, 6, 15, INVALID_SQUARE, 7, INVALID_SQUARE, 8, 17, 26, 35, 44, 53, 62, INVALID_SQUARE, 16,
            25, 34, 43, 52, 61, INVALID_SQUARE, 24, 33, 42, 51, 60, INVALID_SQUARE, 32, 41, 50, 59, INVALID_SQUARE, 40,
            49, 58, INVALID_SQUARE, 48, 57, INVALID_SQUARE, 56, INVALID_SQUARE
    };

    const int sw_indices[64]{
            0, 4, 7, 11, 16, 22, 29, 37, 0, 8, 12, 17, 23, 30, 38, 46, 0, 13, 18, 24, 31, 39, 47, 54, 0, 19, 25, 32, 40,
            48, 55, 61, 0, 26, 33, 41, 49, 56, 62, 67, 0, 34, 42, 50, 57, 63, 68, 72, 0, 43, 51, 58, 64, 69, 73, 76, 0,
            0, 0, 0, 0, 0, 0, 0
    };
    const int ne_indices[64]{
            0, 2, 5, 9, 14, 20, 27, 77, 3, 6, 10, 15, 21, 28, 36, 44, 7, 11, 16, 22, 29, 37, 45, 52, 12, 17, 23, 30, 38,
            46, 53, 59, 18, 24, 31, 39, 47, 54, 60, 65, 25, 32, 40, 48, 55, 61, 66, 70, 33, 41, 49, 56, 62, 67, 71, 74,
            42, 50, 57, 63, 68, 72, 75, 77
    };
    const Square sw_ne_squares[80]{
            INVALID_SQUARE, 0, INVALID_SQUARE, 1, 8, INVALID_SQUARE, 2, 9, 16, INVALID_SQUARE, 3, 10, 17, 24,
            INVALID_SQUARE, 4, 11, 18, 25, 32, INVALID_SQUARE, 5, 12, 19, 26, 33, 40, INVALID_SQUARE, 6, 13, 20, 27, 34,
            41, 48, INVALID_SQUARE, 7, 14, 21, 28, 35, 42, 49, 56, INVALID_SQUARE, 15, 22, 29, 36, 43, 50, 57,
            INVALID_SQUARE, 23, 30, 37, 44, 51, 58, INVALID_SQUARE, 31, 38, 45, 52, 59, INVALID_SQUARE, 39, 46, 53, 60,
            INVALID_SQUARE, 47, 54, 61, INVALID_SQUARE, 55, 62, INVALID_SQUARE, 63, INVALID_SQUARE
    };

    const int* indices[]{s_indices, n_indices, e_indices, w_indices, se_indices, nw_indices, sw_indices, ne_indices};
    const Square* squares[]{s_n_squares, e_w_squares, se_nw_squares, sw_ne_squares};

    // Works the same way as directional data, but does not require directions. knight_indices[square] will point you
    // to a section of knight_data that lists all squares that can contain attacking knights.
    // KNIGHT DATA
    const unsigned int knight_indices[64]{
            0, 3, 7, 12, 17, 22, 27, 31, 34, 38, 43, 50, 57, 64, 71, 76, 80, 85, 92, 101, 110, 119, 128, 135, 140, 145,
            152, 161, 170, 179, 188, 195, 200, 205, 212, 221, 230, 239, 248, 255, 260, 265, 272, 281, 290, 299, 308,
            315, 320, 324, 329, 336, 343, 350, 357, 362, 366, 369, 373, 378, 383, 388, 393, 397
    };
    const Square knight_data[400]{
            10, 17, INVALID_SQUARE, 16, 11, 18, INVALID_SQUARE, 8, 17, 12, 19, INVALID_SQUARE, 9, 18, 13, 20,
            INVALID_SQUARE, 10, 19, 14, 21, INVALID_SQUARE, 11, 20, 15, 22, INVALID_SQUARE, 12, 21, 23, INVALID_SQUARE,
            13, 22, INVALID_SQUARE, 2, 18, 25, INVALID_SQUARE, 24, 3, 19, 26, INVALID_SQUARE, 0, 16, 25, 4, 20, 27,
            INVALID_SQUARE, 1, 17, 26, 5, 21, 28, INVALID_SQUARE, 2, 18, 27, 6, 22, 29, INVALID_SQUARE, 3, 19, 28, 7,
            23, 30, INVALID_SQUARE, 4, 20, 29, 31, INVALID_SQUARE, 5, 21, 30, INVALID_SQUARE, 1, 10, 26, 33,
            INVALID_SQUARE, 0, 32, 2, 11, 27, 34, INVALID_SQUARE, 1, 8, 24, 33, 3, 12, 28, 35, INVALID_SQUARE, 2, 9, 25,
            34, 4, 13, 29, 36, INVALID_SQUARE, 3, 10, 26, 35, 5, 14, 30, 37, INVALID_SQUARE, 4, 11, 27, 36, 6, 15, 31,
            38, INVALID_SQUARE, 5, 12, 28, 37, 7, 39, INVALID_SQUARE, 6, 13, 29, 38, INVALID_SQUARE, 9, 18, 34, 41,
            INVALID_SQUARE, 8, 40, 10, 19, 35, 42, INVALID_SQUARE, 9, 16, 32, 41, 11, 20, 36, 43, INVALID_SQUARE, 10,
            17, 33, 42, 12, 21, 37, 44, INVALID_SQUARE, 11, 18, 34, 43, 13, 22, 38, 45, INVALID_SQUARE, 12, 19, 35, 44,
            14, 23, 39, 46, INVALID_SQUARE, 13, 20, 36, 45, 15, 47, INVALID_SQUARE, 14, 21, 37, 46, INVALID_SQUARE, 17,
            26, 42, 49, INVALID_SQUARE, 16, 48, 18, 27, 43, 50, INVALID_SQUARE, 17, 24, 40, 49, 19, 28, 44, 51,
            INVALID_SQUARE, 18, 25, 41, 50, 20, 29, 45, 52, INVALID_SQUARE, 19, 26, 42, 51, 21, 30, 46, 53,
            INVALID_SQUARE, 20, 27, 43, 52, 22, 31, 47, 54, INVALID_SQUARE, 21, 28, 44, 53, 23, 55, INVALID_SQUARE, 22,
            29, 45, 54, INVALID_SQUARE, 25, 34, 50, 57, INVALID_SQUARE, 24, 56, 26, 35, 51, 58, INVALID_SQUARE, 25, 32,
            48, 57, 27, 36, 52, 59, INVALID_SQUARE, 26, 33, 49, 58, 28, 37, 53, 60, INVALID_SQUARE, 27, 34, 50, 59, 29,
            38, 54, 61, INVALID_SQUARE, 28, 35, 51, 60, 30, 39, 55, 62, INVALID_SQUARE, 29, 36, 52, 61, 31, 63,
            INVALID_SQUARE, 30, 37, 53, 62, INVALID_SQUARE, 33, 42, 58, INVALID_SQUARE, 32, 34, 43, 59, INVALID_SQUARE,
            33, 40, 56, 35, 44, 60, INVALID_SQUARE, 34, 41, 57, 36, 45, 61, INVALID_SQUARE, 35, 42, 58, 37, 46, 62,
            INVALID_SQUARE, 36, 43, 59, 38, 47, 63, INVALID_SQUARE, 37, 44, 60, 39, INVALID_SQUARE, 38, 45, 61,
            INVALID_SQUARE, 41, 50, INVALID_SQUARE, 40, 42, 51, INVALID_SQUARE, 41, 48, 43, 52, INVALID_SQUARE, 42, 49,
            44, 53, INVALID_SQUARE, 43, 50, 45, 54, INVALID_SQUARE, 44, 51, 46, 55, INVALID_SQUARE, 45, 52, 47,
            INVALID_SQUARE, 46, 53, 64
    };

    // May as well speed up king move generation for ~half a kb
    // KING DATA
    const unsigned int king_indices[64]{
            0, 4, 10, 16, 22, 28, 34, 40, 44, 50, 59, 68, 77, 86, 95, 104, 110, 116, 125, 134, 143, 152, 161, 170, 176,
            182, 191, 200, 209, 218, 227, 236, 242, 248, 257, 266, 275, 284, 293, 302, 308, 314, 323, 332, 341, 350,
            359, 368, 374, 380, 389, 398, 407, 416, 425, 434, 440, 444, 450, 456, 462, 468, 474, 480
    };
    const Square king_data[484]{
            1, 9, 8, INVALID_SQUARE, 0, 8, 2, 10, 9, INVALID_SQUARE, 1, 9, 3, 11, 10, INVALID_SQUARE, 2, 10, 4, 12, 11,
            INVALID_SQUARE, 3, 11, 5, 13, 12, INVALID_SQUARE, 4, 12, 6, 14, 13, INVALID_SQUARE, 5, 13, 7, 15, 14,
            INVALID_SQUARE, 6, 14, 15, INVALID_SQUARE, 1, 9, 17, 0, 16, INVALID_SQUARE, 0, 8, 16, 2, 10, 18, 1, 17,
            INVALID_SQUARE, 1, 9, 17, 3, 11, 19, 2, 18, INVALID_SQUARE, 2, 10, 18, 4, 12, 20, 3, 19, INVALID_SQUARE, 3,
            11, 19, 5, 13, 21, 4, 20, INVALID_SQUARE, 4, 12, 20, 6, 14, 22, 5, 21, INVALID_SQUARE, 5, 13, 21, 7, 15, 23,
            6, 22, INVALID_SQUARE, 6, 14, 22, 7, 23, INVALID_SQUARE, 9, 17, 25, 8, 24, INVALID_SQUARE, 8, 16, 24, 10,
            18, 26, 9, 25, INVALID_SQUARE, 9, 17, 25, 11, 19, 27, 10, 26, INVALID_SQUARE, 10, 18, 26, 12, 20, 28, 11,
            27, INVALID_SQUARE, 11, 19, 27, 13, 21, 29, 12, 28, INVALID_SQUARE, 12, 20, 28, 14, 22, 30, 13, 29,
            INVALID_SQUARE, 13, 21, 29, 15, 23, 31, 14, 30, INVALID_SQUARE, 14, 22, 30, 15, 31, INVALID_SQUARE, 17, 25,
            33, 16, 32, INVALID_SQUARE, 16, 24, 32, 18, 26, 34, 17, 33, INVALID_SQUARE, 17, 25, 33, 19, 27, 35, 18, 34,
            INVALID_SQUARE, 18, 26, 34, 20, 28, 36, 19, 35, INVALID_SQUARE, 19, 27, 35, 21, 29, 37, 20, 36,
            INVALID_SQUARE, 20, 28, 36, 22, 30, 38, 21, 37, INVALID_SQUARE, 21, 29, 37, 23, 31, 39, 22, 38,
            INVALID_SQUARE, 22, 30, 38, 23, 39, INVALID_SQUARE, 25, 33, 41, 24, 40, INVALID_SQUARE, 24, 32, 40, 26, 34,
            42, 25, 41, INVALID_SQUARE, 25, 33, 41, 27, 35, 43, 26, 42, INVALID_SQUARE, 26, 34, 42, 28, 36, 44, 27, 43,
            INVALID_SQUARE, 27, 35, 43, 29, 37, 45, 28, 44, INVALID_SQUARE, 28, 36, 44, 30, 38, 46, 29, 45,
            INVALID_SQUARE, 29, 37, 45, 31, 39, 47, 30, 46, INVALID_SQUARE, 30, 38, 46, 31, 47, INVALID_SQUARE, 33, 41,
            49, 32, 48, INVALID_SQUARE, 32, 40, 48, 34, 42, 50, 33, 49, INVALID_SQUARE, 33, 41, 49, 35, 43, 51, 34, 50,
            INVALID_SQUARE, 34, 42, 50, 36, 44, 52, 35, 51, INVALID_SQUARE, 35, 43, 51, 37, 45, 53, 36, 52,
            INVALID_SQUARE, 36, 44, 52, 38, 46, 54, 37, 53, INVALID_SQUARE, 37, 45, 53, 39, 47, 55, 38, 54,
            INVALID_SQUARE, 38, 46, 54, 39, 55, INVALID_SQUARE, 41, 49, 57, 40, 56, INVALID_SQUARE, 40, 48, 56, 42, 50,
            58, 41, 57, INVALID_SQUARE, 41, 49, 57, 43, 51, 59, 42, 58, INVALID_SQUARE, 42, 50, 58, 44, 52, 60, 43, 59,
            INVALID_SQUARE, 43, 51, 59, 45, 53, 61, 44, 60, INVALID_SQUARE, 44, 52, 60, 46, 54, 62, 45, 61,
            INVALID_SQUARE, 45, 53, 61, 47, 55, 63, 46, 62, INVALID_SQUARE, 46, 54, 62, 47, 63, INVALID_SQUARE, 49, 57,
            48, INVALID_SQUARE, 48, 56, 50, 58, 49, INVALID_SQUARE, 49, 57, 51, 59, 50, INVALID_SQUARE, 50, 58, 52, 60,
            51, INVALID_SQUARE, 51, 59, 53, 61, 52, INVALID_SQUARE, 52, 60, 54, 62, 53, INVALID_SQUARE, 53, 61, 55, 63,
            54, INVALID_SQUARE, 54, 62, 55, 64
    };

    // Returns the direction between two squares, or INVALID_DIR if it's none of N, E, S, W, NE, NW, SE, SW
    Direction getDirection(Square from, Square to) {
        int dist = from - to;

        // Multiple of 8 is N, of -8 is S (wrapping impossible)
        if (dist % 8 == 0) {
            return dist > 0 ? NORTH : SOUTH;
        }

        // Must check vertical/horizontal distance to prevent wrapping
        int vDist = verticalDistance(from, to);
        if (vDist == horizontalDistance(from, to)) {
            // Multiple of 7 is NE, -7 is SW
            if (dist % 7 == 0) {
                return dist > 0 ? NORTHEAST : SOUTHWEST;
            }

            // Multiple of 9 is NW, -9 is SE
            if (dist % 9 == 0) {
                return dist > 0 ? NORTHWEST : SOUTHEAST;
            }
        } else if (vDist == 0 && dist < 8) { // Dist of 1-7 is E if negative, W if positive
            return dist < 0 ? EAST : WEST;
        }

        return INVALID_DIRECTION;
    }
}