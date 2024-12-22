#include "defs.h"

static void checkUp(S_SEARCHINFO *info) {
    // check if time up or interrupt from GUI
    if (info->timeset == true && getTimeMs() > info->stoptime) info->stopped = true;
    readInput(info);
}

static void pickNextMove(int moveNum, S_MOVELIST *list) {
    S_MOVE temp;
    int bestScore = 0;
    int bestNum = moveNum;

    for (int index = moveNum; index < list->count; index++) {
        if (list->moves[index].score > bestScore) {
            bestScore = list->moves[index].score;
            bestNum = index;
        }
    }
    temp = list->moves[moveNum];
    list->moves[moveNum] = list->moves[bestNum];
    list->moves[bestNum] = temp;
}

static bool isRepetition(const S_BOARD *pos) {
    for (int index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; index++) {
        ASSERT(index >= 0 && index < MAX_GAME_MOVES);
        if (pos->posKey == pos->history[index].posKey) return true;
    }
    return false;
}

static void clearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {
    for (int index = 0; index < 13; index++) {
        for (int index2 = 0; index2 < BRD_SQ_NUM; index2++) {
            pos->searchHistory[index][index2] = 0;
        }
    }

    for (int index = 0; index < 2; index++) {
        for (int index2 = 0; index2 < MAX_DEPTH; index2++) {
            pos->searchKillers[index][index2] = 0;
        }
    }

    pos->hashTable->overWrite = 0;
    pos->hashTable->hit = 0;
    pos->hashTable->cut = 0;

    pos->ply = 0;

    info->stopped = 0;
    info->nodes = 0;
    info->fh = 0;
    info->fhf = 0;
}

// Searches only the captures to resolve the "Horizon Effect"
static int quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {
    ASSERT(checkBoard(pos));
    ASSERT(beta > alpha);
    
    if ((info->nodes & 2047) == 0) checkUp(info);
    
    info->nodes++;

    if (isRepetition(pos) || pos->fiftyMove >= 100) return 0;

    if (pos->ply > MAX_DEPTH - 1) return evalPosition(pos);

    int score = evalPosition(pos);
    ASSERT(score > -INFINITE && score < INFINITE);

    // Standing Pat
    if (score >= beta) return beta;

    if (score > alpha) alpha = score;

    S_MOVELIST list[1];
    generateAllCaps(pos, list);

    int legal = 0;
    int oldAlpha = alpha;
    
    score = -INFINITE;

    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        pickNextMove(moveNum, list);

        if (!makeMove(pos, list->moves[moveNum].move)) continue;

        legal++;
        score = -quiescence(-beta, -alpha, pos, info);
        takeMove(pos);

        if (info->stopped == true) return 0;

        if (score > alpha) {
            if (score >= beta) {
                if (legal == 1) info->fhf++;
                info->fh++;
                return beta;
            }
            alpha = score;
        }
    }


    ASSERT(alpha >= oldAlpha)

    return alpha;
}

static int alphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, bool doNULL) {
    ASSERT(checkBoard(pos));
    ASSERT(beta > alpha);
    ASSERT(depth >= 0);

    if (depth <= 0) return quiescence(alpha, beta, pos, info);

    if ((info->nodes & 2047) == 0) checkUp(info);

    info->nodes++;

    if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) return 0;

    if (pos->ply > MAX_DEPTH - 1) return evalPosition(pos);

    bool inCheck = sqAttacked(pos->kingSq[pos->side], (pos->side) ^ 1, pos);
    if (inCheck) depth++;

    int score = -INFINITE;
    int pvMove = NOMOVE;

    if (probeHashEntry(pos, &pvMove, &score, alpha, beta, depth) == true) {
        pos->hashTable->cut++;
        return score;
    }

    // Null Move Forward Pruning
    if (doNULL && !inCheck && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
        makeNullMove(pos);
        score = -alphaBeta(-beta, -beta + 1, depth - 4, pos, info, false);
        takeNullMove(pos);

        if (info->stopped) return 0;

        if (score >= beta && abs(score) < ISMATE) return beta;
    }

    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    int legal = 0;
    int oldAlpha = alpha;
    int bestMove = NOMOVE;

    int bestScore = -INFINITE;
    score = -INFINITE;

    // Ordering the principal variation
    if (pvMove != NOMOVE) {
        for (int moveNum = 0; moveNum < list->count; moveNum++) {
            if (list->moves[moveNum].move == pvMove) {
                list->moves[moveNum].score = 2000000;
                break;
            }
        }
    }

    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        pickNextMove(moveNum, list);

        if (!makeMove(pos, list->moves[moveNum].move)) continue;

        legal++;
        score = -alphaBeta(-beta, -alpha, depth - 1, pos, info, true);
        takeMove(pos);

        if (info->stopped) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = list->moves[moveNum].move;

            if (score > alpha) {
                if (score >= beta) {
                    if (legal == 1) info->fhf++;
                    info->fh++;

                    if (!(list->moves[moveNum].move & MFLAGCAP)) {
                        // For beta cut-off, recording in searchKillers for better ordering
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[moveNum].move;
                    }

                    storeHashEntry(pos, bestMove, beta, HFBETA, depth);

                    return beta;
                }
                alpha = score;

                if (!(list->moves[moveNum].move & MFLAGCAP)) {
                    // For alpha improvement, recording in searchHistory for better ordering
                    pos->searchHistory[pos->pieces[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth;
                }
            }
        }
    }

    if (legal == 0) {
        if (inCheck) return -INFINITE + pos->ply;
        else return 0;
    }

    ASSERT(alpha >= oldAlpha);

    if (alpha != oldAlpha) storeHashEntry(pos, bestMove, bestScore, HFEXACT, depth);
    else storeHashEntry(pos, bestMove, alpha, HFALPHA, depth);

    return alpha;
}

void searchPosition(S_BOARD *pos, S_SEARCHINFO *info) {
    int bestMove = NOMOVE;
    int bestScore = -INFINITE;
    int pvMoves = 0;
    int pvNum = 0;

    clearForSearch(pos, info);

    // Iterative Deepening
    for (int currentDepth = 1; currentDepth <= info->depth; currentDepth++) {
        bestScore = alphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, true);

        if (info->stopped) break;

        pvMoves = getPvLine(currentDepth, pos);
        bestMove = pos->pvArray[0];

        if (info->GAME_MODE == UCIMODE) printf("info score cp %d depth %d nodes: %ld time %d ", bestScore, currentDepth, info->nodes, getTimeMs() - info->starttime);
        else if (info->GAME_MODE == XBOARDMODE && info->POST_THINKING == true) printf("%d %d %d %ld ", currentDepth, bestScore, (getTimeMs() - info->starttime) / 10, info->nodes);
        else if (info->POST_THINKING == true) printf("score:%d depth:%d nodes:%ld time:%d(ms) ", bestScore, currentDepth, info->nodes, getTimeMs() - info->starttime);

        if (info->GAME_MODE == UCIMODE || info->POST_THINKING == true) {
            pvMoves = getPvLine(currentDepth, pos);
            printf("pv: ");
            for (pvNum = 0; pvNum < pvMoves; pvNum++) {
                printf(" %s", PrMove(pos->pvArray[pvNum]));
            }
            printf("\n");
        }
    }

    if (info->GAME_MODE == UCIMODE) printf("bestmove %s\n", PrMove(bestMove));
    else if (info->GAME_MODE == XBOARDMODE) {
        printf("move %s\n", PrMove(bestMove));
        makeMove(pos, bestMove);
    } else {
        printf("\n\n***!! Engine makes move %s !!***\n\n", PrMove(bestMove));
        makeMove(pos, bestMove);
        printBoard(pos);
    }
}
