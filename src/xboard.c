#include "defs.h"

int threeFoldRep(const S_BOARD *pos) {
    int rep = 0;
    for (int index = 0; index < pos->hisPly; index++) {
        if (pos->history[index].posKey == pos->posKey) rep++;
    }
    return rep;
}

bool drawMaterial(const S_BOARD *pos) {
    if (pos->pceNum[wP] || pos->pceNum[bP]) return false;
    if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return false;
    if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) return false;
    if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) return false;
    if (pos->pceNum[wN] && pos->pceNum[wB]) return false;
    if (pos->pceNum[bN] && pos->pceNum[bB]) return false;

    return true;
}

bool checkResult(S_BOARD *pos) {
    if (pos->fiftyMove > 100) {
        // Slightly inaccurate condition check here
        printf("1/2-1/2 (fifty move rule (claimed by engine))\n");
        return true;
    }

    if (threeFoldRep(pos) >= 2) {
        printf("1/2-1/2 (3-fold repetition (claimed by engine))\n");
        return true;
    }

    if (drawMaterial(pos) == true) {
        printf("1/2-1/2 (insufficient material (claimed by engine))\n");
        return true;
    }

    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    int found = 0;
    for (int moveNum = 0; moveNum < list->count; moveNum++) {
        if (!makeMove(pos, list->moves[moveNum].move)) continue;

        found++;
        takeMove(pos);
        break;
    }

    if (found != 0) return false;

    int inCheck = sqAttacked(pos->kingSq[pos->side], (pos->side) ^ 1, pos);

    if (inCheck == true) {
        if (pos->side == WHITE) {
            printf("0-1 (black mates (claimed by engine))\n");
            return true;
        } else {
            printf("0-1 (white mates (claimed by engine))\n");
        }
    } else {
        printf("\n1/2-1/2 (stalemate (claimed by engine))\n");
        return true;
    }
    return false;
}

void printOptions() {
    printf("feature ping=1 setboard=1 colors=0 usermove=1\n");
    printf("feature done=1\n");
}

void XBoardLoop(S_BOARD *pos, S_SEARCHINFO *info) {
    info->GAME_MODE = XBOARDMODE;
    info->POST_THINKING = true;

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    printOptions(); // hack for winboard

    int depth = -1, movestogo[2] = {30, 30}, movetime = -1;
    int time = -1, inc = 0;
    int engineSide = BOTH;
    int timeLeft;
    int sec;
    int mps; // moves per session
    int move = NOMOVE;
    int index, score;
    char inBuff[80], command[80];

    engineSide = BLACK;
    parseFEN(START_FEN, pos);
    depth = -1;
    time = -1;

    while (true) {
        fflush(stdout);

        if (pos->side == engineSide && checkResult(pos) == false) {
            info->starttime = getTimeMs();
            info->depth = depth;

            if (time != -1) {
                info->timeset = true;
                time /= movestogo[pos->side];
                time -= 50;
                info->stoptime = info->starttime + time + inc;
            }

            if (depth == -1 || depth > MAX_DEPTH) {
                info->depth = MAX_DEPTH;
            }

            printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n", time, info->starttime, info->stoptime, info->depth, info->timeset, movestogo[pos->side], mps);
            searchPosition(pos, info);

            if (mps != 0) {
                movestogo[(pos->side) ^ 1]--;
                if (movestogo[(pos->side) ^ 1] < 1) movestogo[(pos->side) ^ 1] = mps;
            }

        }

        fflush(stdout);

        memset(&inBuff[0], 0, sizeof(inBuff));
        fflush(stdout);
        if (!fgets(inBuff, 80, stdin)) continue;

        sscanf(inBuff, "%s", command);

        printf("command seen:%s\n", inBuff);

        if (!strcmp(command, "quit")) {
            info->quit = true;
            break;
        }

        if (!strcmp(command, "force")) {
            engineSide = BOTH;
            continue;
        }

        if (!strcmp(command, "protover")) {
            printOptions();
            continue;
        }

        if (!strcmp(command, "sd")) {
            sscanf(inBuff, "sd %d", &depth);
            continue;
        }

        if (!strcmp(command, "st")) {
            sscanf(inBuff, "st %d", &movetime);
            continue;
        }

        if (!strcmp(command, "time")) {
            sscanf(inBuff, "time %d", &time);
            time *= 10;
            printf("DEBUG time:%d\n", time);
            continue;
        }

        if (!strcmp(command, "level")) {
            sec = 0;
            movetime = -1;
            if (sscanf(inBuff, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
                sscanf(inBuff, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
                printf("DEBUG level with:\n");
            } else printf("DEBUG level without:\n");

            timeLeft *= 60000;
            timeLeft += sec * 1000;
            movestogo[0] = movestogo[1] - 30;
            if (mps != 0) movestogo[0] = movestogo[1] = mps;

            time = -1;
            printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps:%d\n", timeLeft, movestogo[0], inc, mps);
            continue;
        }

        if (!strcmp(command, "ping")) {
            printf("pong%s\n", inBuff + 4);
            continue;
        }

        if (!strcmp(command, "new")) {
            engineSide = BLACK;
            parseFEN(START_FEN, pos);
            depth = -1;
            continue;
        }

        if (!strcmp(command, "setboard")) {
            engineSide = BOTH; // effectively in force mode
            parseFEN(inBuff + 9, pos);
            continue;
        }

        if (!strcmp(command, "go")) {
            engineSide = pos->side;
            continue;
        }

        if (!strcmp(command, "usermove")) {
            movestogo[pos->side]--;
            move = parseMove(inBuff + 9, pos);
            if (move == NOMOVE) continue;

            makeMove(pos, move);
            pos->ply = 0;
        }
    }
}

void consoleLoop(S_BOARD *pos, S_SEARCHINFO *info) {
    printf("Starting Chess Engine in Console Mode\n");
    printf("Welcome! Type help for commands\n\n");

    info->GAME_MODE = CONSOLEMODE;
    info->POST_THINKING = true;

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    int depth = MAX_DEPTH, movetime = 3000;
    int engineSide = BOTH;
    int move = NOMOVE;
    char inBuff[80], command[80];

    engineSide = BLACK;
    parseFEN(START_FEN, pos);

    while (true) {
        fflush(stdout);

        if (pos->side == engineSide && checkResult(pos) == false) {
            info->starttime = getTimeMs();
            info->depth = depth;

            if (movetime != 0) {
                info->timeset = true;
                info->stoptime = info->starttime + movetime;
            }

            searchPosition(pos, info);
        }

        printf("\nEngine > ");

        fflush(stdout);
        
        memset(&inBuff[0], 0, sizeof(inBuff));
        fflush(stdout);
        if (!fgets(inBuff, 80, stdin)) continue;

        sscanf(inBuff, "%s", command);

        if (!strcmp(command, "help")) {
            printf("Command List:\n");
            printf("quit - quit game\n");
            printf("force - engine will not thinking\n");
            printf("print - show board\n");
            printf("post - show thinking\n");
            printf("nopost - do not show thinking\n");
            printf("new - start new game\n");
            printf("go - set engine thinking\n");
            printf("depth x - set depth to x\n");
            printf("time x - set thinking time to x seconds (depth still applies if set)\n");
            printf("view - show current depth and movetime settings\n");
            printf("setboard x - set position to FEN x\n");
            printf("***NOTE*** - to reset time and depth, set to 0\n");
            printf("enter moves using the Algebraic Notation (ex: b7b8q)\n\n\n");
            continue;
        }

        if (!strcmp(command, "eval")) {
            printBoard(pos);
            printf("Eval: %d\n", evalPosition(pos));
            mirrorBoard(pos);
            printBoard(pos);
            printf("Eval: %d\n", evalPosition(pos));
            mirrorBoard(pos);
            continue;
        }

        if (!strcmp(command, "setboard")) {
            engineSide = BOTH;
            parseFEN(inBuff + 9, pos);
            continue;
        }

        if (!strcmp(command, "quit")) {
            info->quit = true;
            break;
        }

        if (!strcmp(command, "post")) {
            info->POST_THINKING = true;
            continue;
        }

        if (!strcmp(command, "print")) {
            printBoard(pos);
            continue;
        }

        if (!strcmp(command, "nopost")) {
            info->POST_THINKING = false;
            continue;
        }
        
        if (!strcmp(command, "force")) {
            engineSide = BOTH;
            continue;
        }

        if (!strcmp(command, "view")) {
            if (depth == MAX_DEPTH) printf("depth not set ");
            else printf("depth %d", depth);

            if (movetime != 0) printf(" movetime %ds\n", movetime / 1000);
            else printf(" movetime not set\n");
            
            continue;
        }

        if (!strcmp(command, "depth")) {
            sscanf(inBuff, "depth %d", &depth);
            if (depth == 0) depth == MAX_DEPTH;
            continue;
        }

        if (!strcmp(command, "time")) {
            sscanf(inBuff, "time %d", &movetime);
            movetime *= 1000;
            continue;
        }

        if (!strcmp(command, "new")) {
            engineSide = BLACK;
            parseFEN(START_FEN, pos);
            continue;
        }

        if (!strcmp(command, "go")) {
            engineSide = pos->side;
            continue;
        }

        move = parseMove(inBuff, pos);
        if (move == NOMOVE) {
            printf("Command unknown:%s\n", inBuff);
            continue;
        }
        makeMove(pos, move);
        pos->ply = 0;
    }
}
