#include "defs.h"

bool sqOnBoard(const int sq) {
    return (filesBrd[sq] == OFFBOARD) ? false : true;
}

bool sideValid(const int side) {
    return (side == WHITE || side == BLACK) ? true : false;
}

bool fileRankValid(const int fr) {
    return (fr >= 0 && fr <= 7) ? true : false;
}

bool pieceValidEmpty(const int pce) {
    return (pce >= EMPTY && pce <= bK) ? true : false;
}

bool pieceValid(const int pce) {
    return (pce >= wP && pce <= bK) ? true : false;
}

bool moveListOk(const S_MOVELIST *list, const S_BOARD *pos) {
    if (list->count < 0 || list->count >= MAX_POSITION_MOVES) return false;

    int from = 0, to = 0;
    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        to = TOSQ(list->moves[moveNum].move);
        from = FROMSQ(list->moves[moveNum].move);
        
        if (!sqOnBoard(to) || !sqOnBoard(from)) return false;

        if (!pieceValid(pos->pieces[from])) {
            printBoard(pos);
            return false;
        }
    }

    return true;
}

bool sqIs120(const int sq) {
    return (sq >= 0 && sq < 120);
}

bool pceValidEmptyOffbrd(const int pce) {
    return (pieceValidEmpty(pce) || pce == OFFBOARD);
}
