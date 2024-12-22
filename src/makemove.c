#include "defs.h"

#define HASH_PCE(pce, sq) (pos->posKey ^= (pieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (castleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (sideKey))
#define HASH_EP (pos->posKey ^= (pieceKeys[EMPTY][(pos->enPas)]))

const int castlePermission[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 7, 15, 15, 15, 3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void clearPiece(const int sq, S_BOARD *pos) {
    ASSERT(sqOnBoard(sq));

    int pce = pos->pieces[sq];

    ASSERT(pieceValid(pce));

    int col = pieceCol[pce];
    int t_pceNum = -1;

    HASH_PCE(pce, sq);

    pos->pieces[sq] = EMPTY;
    pos->material[col] -= pieceVal[pce];

    if (pieceBig[pce]) {
        pos->bigPce[col]--;
        if (pieceMaj[pce]) pos->majPce[col]--;
        else pos->minPce[col]--;
    } else {
        CLRBIT(pos->pawns[col], SQ64(sq));
        CLRBIT(pos->pawns[BOTH], SQ64(sq));
    }

    for (int index = 0; index < pos->pceNum[pce]; index++) {
        if (pos->pList[pce][index] == sq) {
            t_pceNum = index;
            break;
        }
    }

    ASSERT(t_pceNum != -1);

    pos->pceNum[pce]--;

    pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];
}

static void addPiece(const int sq, S_BOARD *pos, const int pce) {
    ASSERT(pieceValid(pce));
    ASSERT(sqOnBoard(sq));

    int col = pieceCol[pce];

    HASH_PCE(pce, sq);

    pos->pieces[sq] = pce;

    if (pieceBig[pce]) {
        pos->bigPce[col]++;
        if (pieceMaj[pce]) pos->majPce[col]++;
        else pos->minPce[col]++;
    } else {
        SETBIT(pos->pawns[col], SQ64(sq));
        SETBIT(pos->pawns[BOTH], SQ64(sq));
    }

    pos->material[col] += pieceVal[pce];
    pos->pList[pce][pos->pceNum[pce]++] = sq;
}

static void movePiece(const int from, const int to, S_BOARD *pos) {
    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));

    int pce = pos->pieces[from];
    int col = pieceCol[pce];

#ifdef DEBUG
    bool t_pieceNum = false;
#endif

    HASH_PCE(pce, from);
    pos->pieces[from] = EMPTY;

    HASH_PCE(pce, to);
    pos->pieces[to] = pce;

    if (!pieceBig[pce]) {
        CLRBIT(pos->pawns[col], SQ64(from));
        CLRBIT(pos->pawns[BOTH], SQ64(from));
        SETBIT(pos->pawns[col], SQ64(to));
        SETBIT(pos->pawns[BOTH], SQ64(to));
    }

    for (int index = 0; index < pos->pceNum[pce]; index++) {
        if (pos->pList[pce][index] == from) {
            pos->pList[pce][index] = to;
#ifdef DEBUG
            t_pieceNum = true;
#endif
            break;
        }
    }

    ASSERT(t_pieceNum);
}

bool makeMove(S_BOARD *pos, int move) {
    ASSERT(checkBoard(pos));

    int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = pos->side;

    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
    ASSERT(sideValid(side));
    ASSERT(pieceValid(pos->pieces[from]));

    pos->history[pos->hisPly].posKey = pos->posKey;

    if (move & MFLAGEP) {
        if (side == WHITE) clearPiece(to - 10, pos);
        else clearPiece(to + 10, pos);
    } else if (move & MFLAGCA) {
        switch(to) {
            case C1: movePiece(A1, D1, pos); break;
            case C8: movePiece(A8, D8, pos); break;
            case G1: movePiece(H1, F1, pos); break;
            case G8: movePiece(H8, F8, pos); break;
            default: ASSERT(false); break;
        }
    }

    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->history[pos->hisPly].move = move;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;

    pos->castlePerm &= castlePermission[from];
    pos->castlePerm &= castlePermission[to];
    pos->enPas = NO_SQ;

    HASH_CA;

    int captured = CAPTURED(move);
    pos->fiftyMove++;

    if (captured != EMPTY) {
        ASSERT(pieceValid(captured));
        clearPiece(to, pos);
        pos->fiftyMove = 0;
    }

    pos->hisPly++;
    pos->ply++;

    if (piecePawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;
        if (move & MFLAGPS) {
            if (side == WHITE) {
                pos->enPas = from + 10;
                ASSERT(ranksBrd[pos->enPas] == RANK_3);
            } else {
                pos->enPas = from - 10;
                ASSERT(ranksBrd[pos->enPas] == RANK_6);
            }
            HASH_EP;
        }
    }

    movePiece(from, to, pos);

    int prPce = PROMOTED(move);
    if (prPce != EMPTY) {
        ASSERT(pieceValid(prPce) && !piecePawn[prPce]);
        clearPiece(to, pos);
        addPiece(to, pos, prPce);
    }

    if (pieceKing[pos->pieces[to]]) {
        pos->kingSq[pos->side] = to;
    }

    pos->side ^= 1;
    HASH_SIDE;

    ASSERT(checkBoard(pos));

    if (sqAttacked(pos->kingSq[side], pos->side, pos)) {
        takeMove(pos);
        return false;
    }

    return true;
}

void takeMove(S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    pos->hisPly--;
    pos->ply--;

    int move = pos->history[pos->hisPly].move;
    int from = FROMSQ(move);
    int to = TOSQ(move);

    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));

    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->side ^= 1;
    HASH_SIDE;

    if (MFLAGEP & move) {
        if (pos->side == WHITE) addPiece(to - 10, pos, bP);
        else addPiece(to + 10, pos, wP);
    } else if (MFLAGCA & move) {
        switch(to) {
            case C1: movePiece(D1, A1, pos); break;
            case C8: movePiece(D8, A8, pos); break;
            case G1: movePiece(F1, H1, pos); break;
            case G8: movePiece(F8, H8, pos); break;
            default: ASSERT(false); break;
        }
    }

    movePiece(to, from, pos);
    
    if (pieceKing[pos->pieces[from]]) pos->kingSq[pos->side] = from;

    int captured = CAPTURED(move);
    if (captured != EMPTY) {
        ASSERT(pieceValid(captured));
        addPiece(to, pos, captured);
    }

    if (PROMOTED(move) != EMPTY) {
        ASSERT(pieceValid(PROMOTED(move)) && !piecePawn[PROMOTED(move)]);
        clearPiece(from, pos);
        addPiece(from, pos, (pieceCol[PROMOTED(move)] == WHITE ? wP : bP));
    }

    ASSERT(checkBoard(pos));
}

void makeNullMove(S_BOARD *pos) {
    ASSERT(checkBoard(pos));
    ASSERT(sqAttacked(pos->kingSq[pos->side], (pos->side) ^ 1, pos));

    pos->ply++;
    pos->history[pos->hisPly].posKey = pos->posKey;

    if (pos->enPas != NO_SQ) HASH_EP;

    pos->history[pos->hisPly].move = NOMOVE;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
    pos->enPas = NO_SQ;

    pos->side ^= 1;
    pos->hisPly++;
    HASH_SIDE;

    ASSERT(checkBoard(pos));
}

void takeNullMove(S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    pos->hisPly--;
    pos->ply--;

    if (pos->enPas != NO_SQ) HASH_EP;

    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    if (pos->enPas != NO_SQ) HASH_EP;
    pos->side ^= 1;
    HASH_SIDE;

    ASSERT(checkBoard(pos));
}
