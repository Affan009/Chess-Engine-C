#include "defs.h"

int getPvLine(const int depth, S_BOARD *pos) {
    ASSERT(depth < MAX_DEPTH && depth >= 1);

    int move = probePvMove(pos);
    int count = 0;
    while (move != NOMOVE && count < depth) {
        ASSERT(count < MAX_DEPTH);

        if (moveExists(pos, move)) {
            makeMove(pos, move);
            pos->pvArray[count++] = move;
        } else break;
        move = probePvMove(pos);
    }

    while (pos->ply > 0) {
        takeMove(pos);
    }
    return count;
}

const int hashSize = 0x100000 * 16;

void clearHashTable(S_HASHTABLE *table) {
    for (S_HASHENTRY *tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
        tableEntry->posKey = 0ULL;
        tableEntry->move = NOMOVE;
        tableEntry->depth = 0;
        tableEntry->score = 0;
        tableEntry->flags = 0;
    }
    table->newWrite = 0;
}

void initHashTable(S_HASHTABLE *table) {
    table->numEntries = hashSize / sizeof(S_HASHENTRY);
    table->numEntries -= 2;
    if (table->pTable != NULL) free(table->pTable);
    table->pTable = (S_HASHENTRY *) malloc(table->numEntries * sizeof(S_HASHENTRY));
    clearHashTable(table);
    printf("HashTable init complete with %d entries\n", table->numEntries);
}

void storeHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {
    int index = pos->posKey % pos->hashTable->numEntries;

    ASSERT(index >= 0 && index < pos->hashTable->numEntries);
    ASSERT(depth >= 1 && depth < MAX_DEPTH);
    ASSERT(flags >= HFALPHA && flags <= HFEXACT);
    ASSERT(score >= -INFINITE && score <= INFINITE);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);

    if (pos->hashTable->pTable[index].posKey == 0) pos->hashTable->newWrite++;
    else pos->hashTable->overWrite++;

    if (score > ISMATE) score += pos->ply;
    else if (score < -ISMATE) score -= pos->ply;

    pos->hashTable->pTable[index].move = move;
    pos->hashTable->pTable[index].posKey = pos->posKey;
    pos->hashTable->pTable[index].flags = flags;
    pos->hashTable->pTable[index].score = score;
    pos->hashTable->pTable[index].depth = depth;
}

bool probeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {
    int index = pos->posKey % pos->hashTable->numEntries;
    
    ASSERT(index >= 0 && index < pos->hashTable->numEntries);
    ASSERT(depth >= 1 && depth < MAX_DEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INFINITE && alpha <= INFINITE);
    ASSERT(beta >= - INFINITE && beta <= INFINITE);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);

    if (pos->hashTable->pTable[index].posKey == pos->posKey) {
        *move = pos->hashTable->pTable[index].move;
        if (pos->hashTable->pTable[index].depth >= depth) {
            pos->hashTable->hit++;

            ASSERT(pos->hashTable->pTable[index].depth >= 1 && pos->hashTable->pTable[index].depth < MAX_DEPTH);
            ASSERT(pos->hashTable->pTable[index].flags >= HFALPHA && pos->hashTable->pTable[index].flags <= HFEXACT);

            *score = pos->hashTable->pTable[index].score;
            if (*score > ISMATE) *score -= pos->ply;
            else if (*score < -ISMATE) *score += pos->ply;

            switch(pos->hashTable->pTable[index].flags) {
                ASSERT(*score >= -INFINITE && *score <= INFINITE);

                case HFALPHA:
                    if (*score <= alpha) {
                        *score = alpha;
                        return true;
                    }
                    break;
                case HFBETA:
                    if (*score >= beta) {
                        *score = beta;
                        return true;
                    }
                    break;
                case HFEXACT:
                    return true;
                    break;
                default: ASSERT(false); break;
            }
        }
    }

    return false;
}

int probePvMove(const S_BOARD *pos) {
    int index = pos->posKey % pos->hashTable->numEntries;
    ASSERT(index >= 0 && index < pos->hashTable->numEntries);

    if (pos->hashTable->pTable[index].posKey == pos->posKey) return pos->hashTable->pTable[index].move;

    return NOMOVE;
}
