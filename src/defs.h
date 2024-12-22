#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if (!(n)) { \
printf("%s - Failed\n", #n); \
printf("On %s\n", __DATE__); \
printf("At %s\n", __TIME__); \
printf("In File %s\n", __FILE__); \
printf("At Line %d\n", __LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "Eng 1.0"
#define BRD_SQ_NUM 120

#define MAX_GAME_MOVES 2048
#define MAX_POSITION_MOVES 256
#define MAX_DEPTH 64

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define NOMOVE 0

#define INFINITE 30000
#define ISMATE (INFINITE - MAX_DEPTH)

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK }; // Pieces
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE }; // Files
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE }; // Ranks

enum { WHITE, BLACK, BOTH }; // Piece-type

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 }; // Castle permission index (representing bit position that indicates castling permission)

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
}; // Board cells

enum { UCIMODE, XBOARDMODE, CONSOLEMODE };

typedef struct {

    int move;
    int score; // Used in move evaluation

} S_MOVE;

typedef struct {

    S_MOVE moves[MAX_POSITION_MOVES];
    int count;

} S_MOVELIST;

enum { HFNONE, HFALPHA, HFBETA, HFEXACT };

typedef struct {

    U64 posKey; // Position from where the move is to be taken
    int move; // Move leading to the best line

    int score;
    int depth;
    int flags;

} S_HASHENTRY;

typedef struct {

    S_HASHENTRY *pTable;
    int numEntries;
    
    int newWrite;
    int overWrite;
    int hit;
    int cut;

} S_HASHTABLE;

typedef struct {

    int move; // Storing the move played that chaned the position
    int castlePerm; // Storing castling permission before the move was made
    int enPas; // Storing the En Passant squares before the move was made
    int fiftyMove; // Storing the state of the 50 Move Rule
    U64 posKey; // Storing the unique key of the position before the move was made

} S_POSITION_HIST;

typedef struct {

    int pieces[BRD_SQ_NUM];
    U64 pawns[3]; // Holding the pawn positions in a 64 bit int indexed by piece-type

    int kingSq[2]; // Holding the positions of the 2 kings

    int side; // Storing which side is to play
    int enPas; // Storing any en passant square (initially set to NO_SQ)
    int fiftyMove; // Keeping count for 50 move rule

    int ply; // Storing the number of half moves in the current search
    int hisPly; // Storing the number of half moves played in the total game

    int castlePerm; // Holding the permissions for castling for both W and B

    U64 posKey; // Unique Key to hold the current position of the board

    int pceNum[13]; // Holding number of pieces indexed by piece-type
    int bigPce[2]; // Holding number of big pieces (anything not a pawn) indexed by piece-colour
    int majPce[2]; // Holding number of major pieces (rooks and queens) indexed by piece-colour
    int minPce[2]; // Holding number of minor pieces (knights and bishops) indexed by piece-colour
    int material[2]; // Holding the material value for the sides (White and Black)

    S_POSITION_HIST history[MAX_GAME_MOVES]; // Holding the history of the game positions (can be used to undo the game moves)

    int pList[13][10]; // Piece list (there can be a maximum of 10 pieces of any piece type)
    // Indexed as pList[piece-type][piece-number]

    S_HASHTABLE hashTable[1]; // Transposition Table (Stores Principal Variation as well)
    int pvArray[MAX_DEPTH];

    int searchHistory[13][BRD_SQ_NUM];
    int searchKillers[2][MAX_DEPTH];

} S_BOARD; // Board structure

typedef struct{

    int starttime;
    int stoptime;
    int depth;
    int depthset;
    bool timeset;
    int movestogo;
    bool infinite;

    long nodes;

    int quit;
    bool stopped;

    float fh; // fail high
    float fhf; // fail high first

    int GAME_MODE;
    bool POST_THINKING;

} S_SEARCHINFO; // Structure containing all information about a search

// MACROS

// Utility macros
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])

// To do useful bit operations on bitboards
#define CLRBIT(bb, sq) ((bb) &= clearMask[(sq)])
#define SETBIT(bb, sq) ((bb) |= setMask[(sq)])

// To check for piece type
#define isBQ(p) (pieceBishopQueen[(p)])
#define isRQ(p) (pieceRookQueen[(p)])
#define isKn(p) (pieceKnight[(p)])
#define isKi(p) (pieceKing[(p)])

// To extract info about the from sq, to sq, captured piece, promoted piece etc. from the move
#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m) >> 7) & 0x7F)
#define CAPTURED(m) (((m) >> 14) & 0xF)
#define PROMOTED(m) (((m) >> 20) & 0xF)

// Move flags to determine if any action of interest was taken in a move
#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define MIRROR64(sq) (mirror64[(sq)])

// GLOBAL VARIABLES
extern int Sq120ToSq64[BRD_SQ_NUM]; // Holding the correspondence from 120 Array Board to 64 Array Board
extern int Sq64ToSq120[64]; // Holding the correspondence from 64 Array Board to 120 Array Board

extern U64 setMask[64];
extern U64 clearMask[64];

extern U64 pieceKeys[13][120];
extern U64 sideKey;
extern U64 castleKeys[16];

extern char pceChar[];
extern char sideChar[];
extern char rankChar[];
extern char fileChar[];
// Used for printing the board to the screen

extern int pieceBig[13];
extern int pieceMaj[13];
extern int pieceMin[13];
extern int pieceVal[13];
extern int pieceCol[13];

// Correspondence between the squares and their files and ranks respectively
extern int filesBrd[BRD_SQ_NUM];
extern int ranksBrd[BRD_SQ_NUM];

extern int piecePawn[13];
extern int pieceKnight[13];
extern int pieceKing[13];
extern int pieceRookQueen[13];
extern int pieceBishopQueen[13];
extern int pieceSlides[13];

extern int mirror64[64];

extern U64 fileBBMask[8];
extern U64 rankBBMask[8];

extern U64 blackPassedMask[64];
extern U64 whitePassedMask[64];
extern U64 isolatedMask[64];

// FUNCTIONS

// init.c
extern void allInit();

// bitboard.c
extern void printBitBoard(U64 bb);
extern int popBit(U64 *bb);
extern int countBits(U64 b);

// hashkeys.c
extern U64 generatePosKey(const S_BOARD *pos);

// board.c
extern void resetBoard(S_BOARD *pos);
extern int parseFEN(char *fen, S_BOARD *pos);
extern void printBoard(const S_BOARD *pos);
extern void mirrorBoard(S_BOARD *pos);
extern void updateListMaterial(S_BOARD *pos);
extern bool checkBoard(const S_BOARD *pos);

// utility.c
extern int FRtoSq(int f, int r); // Calculates the board index in 120 Array Board using the file and the rank

// attack.c
extern bool sqAttacked(const int sq, const int side, const S_BOARD *pos);

// io.c
extern char *PrSq(const int sq);
extern char *PrMove(const int move);
extern int parseMove(char *ptrChar, S_BOARD *pos);
extern void printMoveList(const S_MOVELIST *list);

// validate.c
extern bool sqOnBoard(const int sq);
extern bool sideValid(const int side);
extern bool fileRankValid(const int fr);
extern bool pieceValidEmpty(const int pce);
extern bool pieceValid(const int pce);
extern bool moveListOk(const S_MOVELIST *list, const S_BOARD *pos);
extern bool sqIs120(const int sq);
extern bool pceValidEmptyOffbrd(const int pce);

// movegen.c
extern void generateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern bool moveExists(S_BOARD *pos, const int move);
extern int initMVVLVA();
extern void generateAllCaps(const S_BOARD *pos, S_MOVELIST *list);

// makemove.c
extern bool makeMove(S_BOARD *pos, int move);
extern void takeMove(S_BOARD *pos);
extern void makeNullMove(S_BOARD *pos);
extern void takeNullMove(S_BOARD *pos);

// perft.c
extern void perftTest(int depth, S_BOARD *pos);

// search.c
extern void searchPosition(S_BOARD *pos, S_SEARCHINFO *info);

//misc.c
extern int getTimeMs();
extern void readInput(S_SEARCHINFO *info);

// pvtable.c
extern void initHashTable(S_HASHTABLE *table);
extern void storeHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);
extern bool probeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth);
extern int probePvMove(const S_BOARD *pos);
extern void clearHashTable(S_HASHTABLE *table);
extern int getPvLine(const int depth, S_BOARD *pos);

// evaluate.c
extern int evalPosition(const S_BOARD *pos);

// uci.c
extern void UCILoop(S_BOARD *pos, S_SEARCHINFO *info);

// xboard.c
extern void XBoardLoop(S_BOARD *pos, S_SEARCHINFO *info);
extern void consoleLoop(S_BOARD *pos, S_SEARCHINFO *info);

#endif
