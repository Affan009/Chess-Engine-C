#include "defs.h"

#define INPUTBUFFER 400 * 6

void parseGo(char *line, S_SEARCHINFO *info, S_BOARD *pos) {
    int depth = -1, movestogo = 30, movetime = -1;
    int time = -1, inc = 0;
    char *ptr = NULL;
    info->timeset = false;

    if ((ptr = strstr(line, "infinite")));

    if ((ptr = strstr(line, "binc")) && pos->side == BLACK) inc = atoi(ptr + 5);

    if ((ptr = strstr(line, "winc")) && pos->side == WHITE) inc = atoi(ptr + 5);

    if ((ptr = strstr(line, "wtime")) && pos->side == WHITE) time = atoi(ptr + 6);

    if ((ptr = strstr(line, "btime")) && pos->side == BLACK) time = atoi(ptr + 5);

    if ((ptr = strstr(line, "movestogo"))) movestogo = atoi(ptr + 10);

    if ((ptr = strstr(line, "movetime"))) movetime = atoi(ptr + 9);

    if ((ptr = strstr(line, "depth"))) depth = atoi(ptr + 6);

    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    info->starttime = getTimeMs();
    info->depth = depth;

    if (time != -1) {
        info->timeset = true;
        time /= movestogo;
        time -= 50;
        info->stoptime = info->starttime + time + inc;
    }

    if (depth == -1) info->depth = MAX_DEPTH;

    printf("time: %d start: %d stop: %d depth: %d timeset: %d\n", time, info->starttime, info->stoptime, info->depth, info->timeset);
    searchPosition(pos, info);
}

void parsePosition(char *lineIn, S_BOARD *pos) {
    lineIn += 9;
    char *ptrChar = lineIn;

    if (strncmp(lineIn, "startpos", 8) == 0) parseFEN(START_FEN, pos);
    else {
        ptrChar = strstr(lineIn, "fen");
        if (ptrChar == NULL) parseFEN(START_FEN, pos);
        else {
            ptrChar += 4;
            parseFEN(ptrChar, pos);
        }
    }

    ptrChar = strstr(lineIn, "moves");
    int move;

    if (ptrChar != NULL) {
        ptrChar += 6;
        while (*ptrChar) {
            move = parseMove(ptrChar, pos);
            if (move == NOMOVE) break;
            makeMove(pos, move);
            pos->ply = 0;
            while (*ptrChar && *ptrChar != ' ') ptrChar++;
            ptrChar++;
        }
    }
    printBoard(pos);
}

void UCILoop(S_BOARD *pos, S_SEARCHINFO *info) {
    // Turning the buffers for standard input and output off (to avoid buffering and not miss any communication)
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUTBUFFER];
    printf("id name %s\n", NAME);
    printf("id author Affan\n");
    printf("uciok\n");

    while(true) {
        memset(&line[0], 0, sizeof(line));
        fflush(stdout);

        if (!fgets(line, INPUTBUFFER, stdin)) continue;

        if(line[0] == '\n') continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "position", 8)) parsePosition(line, pos);
        else if (!strncmp(line, "ucinewgame", 10)) parsePosition("position startpos\n", pos);
        else if (!strncmp(line, "go", 2)) parseGo(line, info, pos);
        else if (!strncmp(line, "quit", 4)) {
            info->quit = true;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n", NAME);
            printf("id author Affan\n");
            printf("uciok\n");
        }
        if (info->quit) break;
    }
}

