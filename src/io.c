#include "defs.h"

char *PrSq(const int sq) {
    static char sqStr[3];

    int file = filesBrd[sq];
    int rank = ranksBrd[sq];

    sprintf(sqStr, "%c%c", ('a' + file), ('1' + rank));
    return sqStr;
}

char *PrMove(const int move) {
    static char mvStr[6];

    int ff = filesBrd[FROMSQ(move)];
    int rf = ranksBrd[FROMSQ(move)];
    int ft = filesBrd[TOSQ(move)];
    int rt = ranksBrd[TOSQ(move)];

    int promoted = PROMOTED(move);

    if (promoted) {
        char pchar = 'q';
        if (isKn(promoted)) pchar = 'n';
        else if (isRQ(promoted) && !isBQ(promoted)) pchar = 'r';
        else if (!isRQ(promoted) && isBQ(promoted)) pchar = 'b';

        sprintf(mvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
    } else sprintf(mvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));

    return mvStr;
}

int parseMove(char *ptrChar, S_BOARD *pos) {

    if (ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
    if (ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
    if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
    if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;

    int from = FRtoSq(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = FRtoSq(ptrChar[2] - 'a', ptrChar[3] - '1');

    ASSERT(sqOnBoard(from) && sqOnBoard(to));

    S_MOVELIST list[1];
    generateAllMoves(pos, list);
    int move = 0;
    int promPce = EMPTY;
    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        move = list->moves[moveNum].move;
        if (FROMSQ(move) == from && TOSQ(move) == to) {
            promPce = PROMOTED(move);
            if (promPce != EMPTY) {
                if (isRQ(promPce) && !isBQ(promPce) && ptrChar[4] == 'r') return move;
                else if (!isRQ(promPce) && isBQ(promPce) && ptrChar[4] == 'b') return move;
                else if (isRQ(promPce) && isBQ(promPce) && ptrChar[4] == 'q') return move;
                else if (isKn(promPce) && ptrChar[4] == 'n') return move;
                continue;
            }
            return move;
        }
    }

    return NOMOVE;
}

void printMoveList(const S_MOVELIST *list) {
    int score = 0, move = 0;
    printf("MoveList: \n", list->count);

    for (int index = 0; index < list->count; index++) {
        move = list->moves[index].move;
        score = list->moves[index].score;
        printf("Move: %d > %s (score: %d)\n", index + 1, PrMove(move), score);
    }
    printf("MoveList Total %d Moves: \n\n", list->count);
}
