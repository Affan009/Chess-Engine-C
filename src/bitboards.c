#include "defs.h"

const int BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2, 
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52, 
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28, 
    58, 20, 37, 17, 36, 8
};

// Returns the index of the least significant bit in the bitboard and then sets it to 0
// Generally used when generating moves
int popBit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

// Counts and returns the number of bits which are 1 in a bitboard
int countBits(U64 b) {
    int r;
    for (r = 0; b; r++, b &= b - 1);
    return r;
}

void printBitBoard(U64 bb) {

    U64 shift = 1ULL;
    int sq = 0, sq64 = 0;

    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = FRtoSq(file, rank);
            sq64 = SQ64(sq);

            if ((shift << sq64) & bb) {
                printf("X");
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
    printf("\n");
}
