#include "defs.h"

#define MOVE(f, t, ca, pro, fl) ((f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))
#define SQOFFBOARD(sq) (filesBrd[(sq)] == OFFBOARD)

const int loopSlidePce[8] = {wB, wR, wQ, 0, bB, bR, bQ, 0};
const int loopSlideIndex[2] = {0, 4};

const int loopNonSlidePce[6] = {wN, wK, 0, bN, bK, 0};
const int loopNonSlideIndex[2] = {0, 3};

const int pceDir[13][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {-8, -19, -21, -12, 8, 19, 21, 12},
    {-9, -11, 11, 9, 0, 0, 0, 0},
    {-1, -10, 1, 10, 0, 0, 0, 0},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {-8, -19, -21, -12, 8, 19, 21, 12},
    {-9, -11, 11, 9, 0, 0, 0, 0},
    {-1, -10, 1, 10, 0, 0, 0, 0},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {-1, -10, 1, 10, -9, -11, 11, 9}
};
// Directions for pawns not set as it won't be used for that
const int numDir[13] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};

const int victimScore[13] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
static int MVVLVAScores[13][13];

int initMVVLVA() {
    for (int attacker = wP; attacker <= bK; attacker++) {
        for (int victim = wP; victim <= bK; victim++) {
            MVVLVAScores[victim][attacker] = victimScore[victim] + 6 - (victimScore[attacker] / 100);
        }
    }
}

bool moveExists(S_BOARD *pos, const int move) {
    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        if (!makeMove(pos, list->moves[moveNum].move)) continue;

        takeMove(pos);
        if (list->moves[moveNum].move == move) return true;
    }
    return false;
}

static void addQuietMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    ASSERT(sqOnBoard(FROMSQ(move)));
    ASSERT(sqOnBoard(TOSQ(move)));

    list->moves[list->count].move = move;

    if (pos->searchKillers[0][pos->ply] == move) list->moves[list->count].score = 900000;
    else if (pos->searchKillers[1][pos->ply] == move) list->moves[list->count].score = 800000;
    else list->moves[list->count].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
    
    list->count++;
}

static void addCaptureMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    ASSERT(sqOnBoard(FROMSQ(move)));
    ASSERT(sqOnBoard(TOSQ(move)));
    ASSERT(pieceValid(CAPTURED(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = MVVLVAScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;
    list->count++;
}

static void addEnPassantMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    ASSERT(sqOnBoard(FROMSQ(move)));
    ASSERT(sqOnBoard(TOSQ(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = 105 + 1000000;
    list->count++;
}

static void addPawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list, int side) {
    ASSERT(pieceValidEmpty(cap));
    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));

    int preProRank = (side == WHITE) ? RANK_7 : RANK_2;
    if (ranksBrd[from] == preProRank) {
        addCaptureMove(pos, MOVE(from, to, cap, (side == WHITE) ? wQ : bQ, 0), list);
        addCaptureMove(pos, MOVE(from, to, cap, (side == WHITE) ? wR : bR, 0), list);
        addCaptureMove(pos, MOVE(from, to, cap, (side == WHITE) ? wB : bB, 0), list);
        addCaptureMove(pos, MOVE(from, to, cap, (side == WHITE) ? wN : bN, 0), list);
    } else addCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
}

static void addPawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list, int side) {
    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
    
    int preProRank = (side == WHITE) ? RANK_7 : RANK_2;
    if (ranksBrd[from] == preProRank) {
        addQuietMove(pos, MOVE(from, to, EMPTY, (side == WHITE) ? wQ : bQ, 0), list);
        addQuietMove(pos, MOVE(from, to, EMPTY, (side == WHITE) ? wR : bR, 0), list);
        addQuietMove(pos, MOVE(from, to, EMPTY, (side == WHITE) ? wB : bB, 0), list);
        addQuietMove(pos, MOVE(from, to, EMPTY, (side == WHITE) ? wN : bN, 0), list);
    } else addQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
}

void generateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(checkBoard(pos));
    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0, t_sq = 0;
    int pceNum = 0;

    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    // Movegen for pawns
    int pawn = (side == WHITE) ? wP : bP;
    int sideMove = (side == WHITE) ? 1 : -1;
    
    for (pceNum = 0; pceNum < pos->pceNum[pawn]; pceNum++) {
        sq = pos->pList[pawn][pceNum];
        ASSERT(sqOnBoard(sq));

        if (pos->pieces[sq + sideMove * 10] == EMPTY) {
        addPawnMove(pos, sq, sq + sideMove * 10, list, side);
                if (ranksBrd[sq] == ((side == WHITE) ? RANK_2 : RANK_7) && pos->pieces[sq + sideMove * 20] == EMPTY) {
                addQuietMove(pos, MOVE(sq, (sq + sideMove * 20), EMPTY, EMPTY, MFLAGPS), list);
            }
        }

        if (!SQOFFBOARD(sq + sideMove * 9) && pieceCol[pos->pieces[sq + sideMove * 9]] == (side ^ 1)) addPawnCapMove(pos, sq, sq + sideMove * 9, pos->pieces[sq + sideMove * 9], list, side);

        if (!SQOFFBOARD(sq + sideMove * 11) && pieceCol[pos->pieces[sq + sideMove * 11]] == (side ^ 1)) addPawnCapMove(pos, sq, sq + sideMove * 11, pos->pieces[sq + sideMove * 11], list, side);

        if (pos->enPas != NO_SQ) {
            if ((sq + sideMove * 9) == pos->enPas) addEnPassantMove(pos, MOVE(sq, sq + sideMove * 9, EMPTY, EMPTY, MFLAGEP), list);

            if ((sq + sideMove * 11) == pos->enPas) addEnPassantMove(pos, MOVE(sq, sq + sideMove * 11, EMPTY, EMPTY, MFLAGEP), list);
        }
    }

    // Checking for castling moves (separately for WHITE and BLACK)
    if (side == WHITE) {
        if (pos->castlePerm & WKCA) {
            if (pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
                if (!sqAttacked(E1, BLACK, pos) && !sqAttacked(F1, BLACK, pos)) {
                    addQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

        if (pos->castlePerm & WQCA) {
            if (pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
                if (!sqAttacked(E1, BLACK, pos) && !sqAttacked(D1, BLACK, pos)) {
                    addQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
    } else {
        if (pos->castlePerm & BKCA) {
            if (pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
                if (!sqAttacked(E8, WHITE, pos) && !sqAttacked(F8, WHITE, pos)) {
                    addQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

        if (pos->castlePerm & BQCA) {
            if (pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
                if (!sqAttacked(E8, WHITE, pos) && !sqAttacked(D8, WHITE, pos)) {
                    addQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
    }

    // Movegen for sliding pieces
    pceIndex = loopSlideIndex[side];
    pce = loopSlidePce[pceIndex++];
    while (pce != 0) {
        ASSERT(pieceValid(pce));
       
        for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
            sq = pos->pList[pce][pceNum];
            ASSERT(sqOnBoard(sq));

            for (index = 0; index < numDir[pce]; index++) {
                dir = pceDir[pce][index];
                t_sq = sq + dir;

                while (!SQOFFBOARD(t_sq)) {
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (pieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            addCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    addQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                    t_sq += dir;
                }
            }
        }

        pce = loopSlidePce[pceIndex++];
    }

    // Movegen for non-sliding pieces
    pceIndex = loopNonSlideIndex[side];
    pce = loopNonSlidePce[pceIndex++];
    while (pce != 0) {
        ASSERT(pieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
            sq = pos->pList[pce][pceNum];
            ASSERT(sqOnBoard(sq));

            for (index = 0; index < numDir[pce]; index++) {
                dir = pceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq)) continue;

                if (pos->pieces[t_sq] != EMPTY) {
                    if (pieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        addCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
                addQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            }
        }

        pce = loopNonSlidePce[pceIndex++];
    }

    ASSERT(moveListOk(list, pos));
}

void generateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(checkBoard(pos));
    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0, t_sq = 0;
    int pceNum = 0;

    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    // Movegen for pawns
    int pawn = (side == WHITE) ? wP : bP;
    int sideMove = (side == WHITE) ? 1 : -1;
    
    for (pceNum = 0; pceNum < pos->pceNum[pawn]; pceNum++) {
        sq = pos->pList[pawn][pceNum];
        ASSERT(sqOnBoard(sq));

        if (!SQOFFBOARD(sq + sideMove * 9) && pieceCol[pos->pieces[sq + sideMove * 9]] == (side ^ 1)) addPawnCapMove(pos, sq, sq + sideMove * 9, pos->pieces[sq + sideMove * 9], list, side);

        if (!SQOFFBOARD(sq + sideMove * 11) && pieceCol[pos->pieces[sq + sideMove * 11]] == (side ^ 1)) addPawnCapMove(pos, sq, sq + sideMove * 11, pos->pieces[sq + sideMove * 11], list, side);

        if (pos->enPas != NO_SQ) {
            if ((sq + sideMove * 9) == pos->enPas) addEnPassantMove(pos, MOVE(sq, sq + sideMove * 9, EMPTY, EMPTY, MFLAGEP), list);

            if ((sq + sideMove * 11) == pos->enPas) addEnPassantMove(pos, MOVE(sq, sq + sideMove * 11, EMPTY, EMPTY, MFLAGEP), list);
        }
    }

    // Movegen for sliding pieces
    pceIndex = loopSlideIndex[side];
    pce = loopSlidePce[pceIndex++];
    while (pce != 0) {
        ASSERT(pieceValid(pce));
       
        for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
            sq = pos->pList[pce][pceNum];
            ASSERT(sqOnBoard(sq));

            for (index = 0; index < numDir[pce]; index++) {
                dir = pceDir[pce][index];
                t_sq = sq + dir;

                while (!SQOFFBOARD(t_sq)) {
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (pieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            addCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    t_sq += dir;
                }
            }
        }

        pce = loopSlidePce[pceIndex++];
    }

    // Movegen for non-sliding pieces
    pceIndex = loopNonSlideIndex[side];
    pce = loopNonSlidePce[pceIndex++];
    while (pce != 0) {
        ASSERT(pieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {
            sq = pos->pList[pce][pceNum];
            ASSERT(sqOnBoard(sq));

            for (index = 0; index < numDir[pce]; index++) {
                dir = pceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq)) continue;

                if (pos->pieces[t_sq] != EMPTY) {
                    if (pieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        addCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
            }
        }

        pce = loopNonSlidePce[pceIndex++];
    }

    ASSERT(moveListOk(list, pos));
}
