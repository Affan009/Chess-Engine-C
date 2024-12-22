#include "defs.h"

const int KnDir[8] = {-8, -19, -21, -12, 8, 19, 21, 12};
const int RkDir[4] = {-1, -10, 1, 10};
const int BiDir[4] = {-9, -11, 11, 9};
const int KiDir[8] = {-1, -10, 1, 10, -9, -11, 11, 9};

bool sqAttacked(const int sq, const int side, const S_BOARD *pos) {
    int pce, t_sq, dir;

    ASSERT(sqOnBoard(sq));
    ASSERT(sideValid(side));
    ASSERT(checkBoard(pos));

    // Checking if a pawn attacks the square
    if (side == WHITE) {
        if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP) return true;
    } else {
        if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP) return true;
    }

    // Checking if a knight attacks the square
    for (int index = 0; index < 8; index++) {
        pce = pos->pieces[sq + KnDir[index]];
        ASSERT(pceValidEmptyOffbrd(pce));
        if (pce != OFFBOARD) {
            if (isKn(pce) && pieceCol[pce] == side) return true;
        }
    }

    // Checking if a rook or a queen attacks the square
    for (int index = 0; index < 4; index++) {
        dir = RkDir[index];
        t_sq = sq + dir;
        ASSERT(sqIs120(t_sq));
        pce = pos->pieces[t_sq];
        ASSERT(pceValidEmptyOffbrd(pce));
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (isRQ(pce) && pieceCol[pce] == side) return true;
                break;
            }
            t_sq += dir;
            ASSERT(sqIs120(t_sq));
            pce = pos->pieces[t_sq];
        }
    }

    // Checking if a bishop or a queen attacks the square
    for (int index = 0; index < 4; index++) {
        dir = BiDir[index];
        t_sq = sq + dir;
        ASSERT(sqIs120(t_sq));
        pce = pos->pieces[t_sq];
        ASSERT(pceValidEmptyOffbrd(pce));
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (isBQ(pce) && pieceCol[pce] == side) return true;
                break;
            }
            t_sq += dir;
            ASSERT(sqIs120(t_sq));
            pce = pos->pieces[t_sq];
        }
    }


    // Checking if the king attacks the square
    for (int index = 0; index < 8; index++) {
        pce = pos->pieces[sq + KiDir[index]];
        ASSERT(pceValidEmptyOffbrd(pce));
        if (pce != OFFBOARD) {
            if (isKi(pce) && pieceCol[pce] == side) return true;
        }
    }

    return false;
}
