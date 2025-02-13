#include "defs.h"

long leafNodes;

void perft(int depth, S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    if (depth == 0) {
        leafNodes++;
        return;
    }

    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        if (!makeMove(pos, list->moves[moveNum].move)) continue;
        perft(depth - 1, pos);
        takeMove(pos);
    }

    return;
}

void perftTest(int depth, S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    printBoard(pos);
    printf("\nStarting Test To Depth: %d\n", depth);
    leafNodes = 0;
    int start = getTimeMs();
    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    int move = 0;
    long cumnodes = 0, oldnodes = 0;
    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        move = list->moves[moveNum].move;
        if (!makeMove(pos, move)) continue;
        cumnodes = leafNodes;
        perft(depth - 1, pos);
        takeMove(pos);
        oldnodes = leafNodes - cumnodes;
        printf("Move %d : %s : %ld\n", moveNum + 1, PrMove(move), oldnodes);
    }

    printf("\nTest Complete: %ld nodes visited in %dms\n", leafNodes, getTimeMs() - start);
    return;
}
