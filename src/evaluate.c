#include "defs.h"

const int pawnIsolated = -10;
const int pawnPassed[8] = {0, 5, 10, 20, 35, 60, 100, 200};

const int rookOpenFile = 5;
const int rookSemiOpenFile = 5;
const int queenOpenFile = 5;
const int queenSemiOpenFile = 3;

const int bishopPair = 30;

const int pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    10, 10, 0, -10, -10, 0, 10, 10,
    5, 0, 0, 5, 5, 0, 0, 5,
    0, 0, 10, 20, 20, 10, 0, 0,
    5, 5, 5, 10, 10, 5, 5, 5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int knightTable[64] = {
    0, -10, 0, 0, 0, 0, -10, 0,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 10, 10, 10, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 5, 0,
    5, 10, 15, 20, 20, 15, 10, 5,
    5, 10, 10, 20, 20, 10, 10, 5,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int bishopTable[64] = {
    0, 0, -10, 0, 0, -10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int rookTable[64] = {
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0, 0, 5, 10, 10, 5, 0, 0
};

const int kingE[64] = {
    -50, -10, 0, 0, 0, 0, -10, -50,
    -10, 0, 10, 10, 10, 10, 0, -10,
    0, 10, 20, 20, 20, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 20, 20, 20, 10, 0,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -50, -10, 0, 0, 0, 0, -10, -50
};

const int kingO[64] = {
    0, 5, 5, -10, -10, 0, 10, 5,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70
};

// adapted from Sjeng chess engine (ver 11.2)
bool materialDraw(const S_BOARD *pos) {
    ASSERT(checkBoard(pos));
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
            if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) return true;
        } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
            if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) return true;
        } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
            if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN])) return true;
        }
    } else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2) return true;
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && ((pos->pceNum[bN] + pos->pceNum[bB] == 1) || (pos->pceNum[bN] + pos->pceNum[bB] == 2))) return true;
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && ((pos->pceNum[wN] + pos->pceNum[wB] == 1) || (pos->pceNum[wN] + pos->pceNum[wB] == 2))) return true;
        }
    }

    return false;
}

#define ENDGAME_MAT (pieceVal[wR] + 2 * pieceVal[wN] + 2 * pieceVal[wP] + pieceVal[wK])

int evalPosition(const S_BOARD *pos) {
    int pce;
    int pceNum;
    int sq;
    int score = pos->material[WHITE] - pos->material[BLACK];

    if (!pos->pceNum[wP] && !pos->pceNum[bP] && materialDraw(pos)) return 0;

    pce = wP;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score += pawnTable[SQ64(sq)];

        if ((isolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) score += pawnIsolated;

        if ((whitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) score += pawnPassed[ranksBrd[sq]];
    }

    pce = bP;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score -= pawnTable[MIRROR64(SQ64(sq))];

        if ((isolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) score -= pawnIsolated;

        if ((blackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) score -= pawnPassed[7 - ranksBrd[sq]];
    }

    pce = wN;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score += knightTable[SQ64(sq)];
    }

    pce = bN;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score -= knightTable[MIRROR64(SQ64(sq))];
    }

    pce = wB;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score += bishopTable[SQ64(sq)];
    }

    pce = bB;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score -= bishopTable[MIRROR64(SQ64(sq))];
    }

    pce = wR;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score += rookTable[SQ64(sq)];

        if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) score += rookOpenFile;
        else if (!(pos->pawns[WHITE] & fileBBMask[filesBrd[sq]])) score += rookSemiOpenFile;
    }

    pce = bR;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));
        score -= rookTable[MIRROR64(SQ64(sq))];

        if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) score -= rookOpenFile;
        else if (!(pos->pawns[BLACK] & fileBBMask[filesBrd[sq]])) score -= rookSemiOpenFile;
    }

    pce = wQ;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));

        if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) score += queenOpenFile;
        else if (!(pos->pawns[WHITE] & fileBBMask[filesBrd[sq]])) score += queenSemiOpenFile;
    }

    pce = bQ;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
        sq = pos->pList[pce][pceNum];
        ASSERT(sqOnBoard(sq));

        if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) score -= queenOpenFile;
        else if (!(pos->pawns[BLACK] & fileBBMask[filesBrd[sq]])) score -= queenSemiOpenFile;
    }

    pce = wK;
    sq = pos->pList[pce][0];
    ASSERT(sqOnBoard(sq));

    if (pos->material[BLACK] <= ENDGAME_MAT) score += kingE[SQ64(sq)];
    else score += kingO[SQ64(sq)];

    pce = bK;
    sq = pos->pList[pce][0];
    ASSERT(sqOnBoard(sq));

    if (pos->material[WHITE] <= ENDGAME_MAT) score -= kingE[MIRROR64(SQ64(sq))];
    else score -= kingO[MIRROR64(SQ64(sq))];

    if (pos->pceNum[wB] >= 2) score += bishopPair;
    if (pos->pceNum[bB] >= 2) score -= bishopPair;

    return (pos->side == WHITE) ? score : -score;
}
