#include "defs.h"

int main() {
    allInit();

    S_BOARD pos[1];
    S_SEARCHINFO info[1];
    info->quit = false;
    initHashTable(pos->hashTable);

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    printf("Welcome to Chess Engine by Affan!\nType 'engine' for Console mode\n");

    char line[256];
    while (true) {
        memset(&line[0], 0, sizeof(line));

        fflush(stdout);
        if (!fgets(line, 256, stdin)) continue;

        if (line[0] == '\n') continue;

        if (!strncmp(line, "uci", 3)) {
            UCILoop(pos, info);
            if (info->quit == true) break;
            continue;
        } else if (!strncmp(line, "xboard", 6)) {
            XBoardLoop(pos, info);
            if (info->quit == true) break;
            continue;
        } else if (!strncmp(line, "engine", 6)) {
            consoleLoop(pos, info);
            if (info->quit == true) break;
            continue;
        } else if (!strncmp(line, "quit", 4)) break;
    }

    free(pos->hashTable->pTable);
    return 0;
}
