#include "defs.h"

U64 generatePosKey(const S_BOARD *pos) {
    U64 finalKey = 0;
    int piece = EMPTY;

    // Hashing in the pieceKeys value for the pieces and squares they are on
    for (int sq = 0; sq < BRD_SQ_NUM; sq++) {
        piece = pos->pieces[sq];
        if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
            ASSERT(piece >= wP && piece <= bK);
            finalKey ^= pieceKeys[piece][sq];
        }
    }

    // Hashing in the sideKey
    if (pos->side == WHITE) finalKey ^= sideKey;

    // Hashing in the enPassant square
    if (pos->enPas != NO_SQ) {
        ASSERT(pos->enPas >= 0 && pos->enPas < BRD_SQ_NUM);
        finalKey ^= pieceKeys[EMPTY][pos->enPas];
    }

    // Hashing in the castle permission key
    ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
    finalKey ^= castleKeys[pos->castlePerm];

    return finalKey;
}
