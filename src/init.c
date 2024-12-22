#include "defs.h"

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 setMask[64];
U64 clearMask[64];

U64 pieceKeys[13][120];
U64 sideKey;
U64 castleKeys[16];

int filesBrd[BRD_SQ_NUM];
int ranksBrd[BRD_SQ_NUM];

U64 fileBBMask[8];
U64 rankBBMask[8];

U64 blackPassedMask[64];
U64 whitePassedMask[64];
U64 isolatedMask[64];

// Generates a 64 bit random integer
static inline U64 rand64() {
    return ((U64)rand() << 32 | (U64)rand());
}

void initEvalMasks() {
    int sq, t_sq, r, f;

    for (sq = 0; sq < 8; sq++) {
        fileBBMask[sq] = 0ULL;
        rankBBMask[sq] = 0ULL;
    }

    for (r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            fileBBMask[f] |= (1ULL << sq);
            rankBBMask[r] |= (1ULL << sq);
        }
    }

    for (sq = 0; sq < 64; sq++) {
        isolatedMask[sq] = 0ULL;
        whitePassedMask[sq] = 0ULL;
        blackPassedMask[sq] = 0ULL;
    }

    for (sq = 0; sq < 64; sq++) {
        t_sq = sq + 8;
        while (t_sq < 64) {
            whitePassedMask[sq] |= (1ULL << t_sq);
            t_sq += 8;
        }

        t_sq = sq - 8;
        while (t_sq >= 0) {
            blackPassedMask[sq] |= (1ULL << t_sq);
            t_sq -= 8;
        }

        if (filesBrd[SQ120(sq)] > FILE_A) {
            isolatedMask[sq] |= fileBBMask[filesBrd[SQ120(sq)] - 1];

            t_sq = sq + 7;
            while (t_sq < 64) {
                whitePassedMask[sq] |= (1ULL << t_sq);
                t_sq += 8;
            }

            t_sq = sq - 9;
            while (t_sq >= 0) {
                blackPassedMask[sq] |= (1ULL << t_sq);
                t_sq -= 8;
            }
        }

        if (filesBrd[SQ120(sq)] < FILE_H) {
            isolatedMask[sq] |= fileBBMask[filesBrd[SQ120(sq)] + 1];
            
            t_sq = sq + 9;
            while (t_sq < 64) {
                whitePassedMask[sq] |= (1ULL << t_sq);
                t_sq += 8;
            }

            t_sq = sq - 7;
            while (t_sq >= 0) {
                blackPassedMask[sq] |= (1ULL << t_sq);
                t_sq -= 8;
            }
        }
    }
}

void initFilesRanksBrd() {
    int sq;

    for (int index = 0; index < BRD_SQ_NUM; index++) {
        filesBrd[index] = OFFBOARD;
        ranksBrd[index] = OFFBOARD;
    }

    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = FRtoSq(file, rank);
            filesBrd[sq] = file;
            ranksBrd[sq] = rank;
        }
    }
}

// Initializer for the hash keys (pos keys) that sets random integer values in the key arrays
void initHashKeys() {
    
    // Sets random values to pieceKeys array
    for (int index = 0; index < 13; index++) {
        for (int index2 = 0; index2 < 120; index2++) {
            pieceKeys[index][index2] = rand64();
        }
    }

    // Sets random value to sideKey
    sideKey = rand64();

    // Sets random values to castleKeys array
    for (int index = 0; index < 16; index++) {
        castleKeys[index] = rand64();
    }
}

// Initializer for the masks used to set and clear bits in the pawn bitboard
void initBitMasks() {
    // Sets each value to 0
    for (int index = 0; index < 64; index++) {
        setMask[index] = 0ULL;
        clearMask[index] = 0ULL;
    }

    // Sets the bit corresponding to the index to 1 in setMask and complement of that to the clearMask
    for (int index = 0; index < 64; index++) {
        setMask[index] |= (1ULL << index);
        clearMask[index] = ~setMask[index];
    }
}

// Initializer for the two board correspondence arrays
void initBoards() {
    
    // Sets each square of the 120 board with the evalue 64 
    // (To ensure the offboard squares do not correspond to any squares on the 64 Board)
    for (int index = 0; index < BRD_SQ_NUM; index++) {
        Sq120ToSq64[index] = 64;
    }

    // Sets the correspondence between the boards
    int sq = 0, sq64 = 0;
    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = FRtoSq(file, rank);
            Sq120ToSq64[sq] = sq64;
            Sq64ToSq120[sq64] = sq;
            sq64++;
        }
    }
    
}

void allInit() {
    initBoards();
    initBitMasks();
    initHashKeys();
    initFilesRanksBrd();
    initEvalMasks();
    initMVVLVA();
}
