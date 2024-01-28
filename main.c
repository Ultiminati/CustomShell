#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "stacks.h"

char msgbuf[257];
regex_t regex;
int tokenIndex = 0;

#define MAX_TOKENS 1024
#define MAX_PATH_SIZE 2048
#define ALIAS_SIZE 256

#define CMDINDEX 0
#define REDIRECTIONINDEX 1
#define REDIRECTIONFILEINDEX 2
#define ARGONE 3
#define ARGTWO 4

struct aliases {
    int size;
    char key[ALIAS_SIZE];
    char value[ALIAS_SIZE];
};

const int parsingTable[65][18][2] = {
        {{-1, -1}, {-1, -1}, {0, 4}, {0, 5}, {0, 6}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 8}, {0, 9}, {2, 1}, {2, 2}, {-1, -1}, {-1, -1}, {2, 3}, {2, 7}, {-1, -1}},
        {{3, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 4}, {0, 12}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 13}, {0, 14}, {0, 15}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 11}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 6}, {1, 6}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 6}, {1, 6}, {1, 6}, {0, 8}, {0, 9}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 16}, {-1, -1}, {2, 17}, {-1, -1}},
        {{1, 7}, {1, 7}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 7}, {1, 7}, {1, 7}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 8}, {1, 8}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 8}, {1, 8}, {1, 8}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 19}, {0, 20}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 18}, {-1, -1}},
        {{1, 15}, {1, 15}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 15}, {1, 15}, {1, 15}, {1, 15}, {1, 15}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 22}, {2, 21}},
        {{1, 19}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 19}, {1, 19}, {1, 19}, {1, 19}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 3}, {0, 25}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 2}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 27}, {0, 28}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 26}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 27}, {0, 28}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 29}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 27}, {0, 28}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 30}, {-1, -1}},
        {{1, 5}, {1, 5}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 5}, {1, 5}, {1, 5}, {0, 8}, {0, 9}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 31}, {-1, -1}},
        {{1, 14}, {1, 14}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 14}, {1, 14}, {1, 14}, {1, 14}, {1, 14}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 32}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 34}, {2, 33}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 36}, {0, 37}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 35}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 38}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 21}, {1, 21}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{1, 1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 10}, {1, 10}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 42}, {2, 41}},
        {{1, 19}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 11}, {1, 11}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 12}, {1, 12}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 13}, {1, 13}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 13}, {1, 13}, {1, 13}, {1, 13}, {1, 13}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 44}, {0, 45}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 43}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 47}, {0, 37}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 46}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 48}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 49}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 18}, {1, 18}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 18}, {1, 18}, {1, 18}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 20}, {1, 20}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 17}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 17}, {1, 17}, {1, 17}, {1, 17}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 51}, {0, 37}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 50}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 52}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 54}, {0, 37}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 53}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 55}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 9}, {1, 9}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 9}, {1, 9}, {1, 9}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 57}, {2, 56}},
        {{1, 19}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 19}, {1, 19}, {1, 19}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 58}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 18}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 16}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 16}, {1, 16}, {1, 16}, {1, 16}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 59}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 18}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 60}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 18}, {1, 18}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{1, 17}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 62}, {0, 37}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 61}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 63}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 16}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 64}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 18}, {1, 18}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 18}, {1, 18}, {1, 18}, {0, 24}, {0, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 40}, {2, 39}},
        {{1, 17}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 17}, {1, 17}, {1, 17}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
        {{1, 16}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {1, 16}, {1, 16}, {1, 16}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}};
//struct token tokens[];
struct intStack *stateStack;
struct Stack *tokenStack;

char lastExecuted[TOKEN_SIZE];
int numberOfProcesses = 0;

void belloFunc(struct token* operationList){
    //bello
    //The "bello" command should display eight information items about the user, respectively:
    //1. Username
    //2. Hostname
    //3. Last Executed Command
    //4. TTY
    //5. Current Shell Name (not myshell)
    //6. Home Location
    //7. Current Time and Date
    //8. Current number of processes being executed (just as everything in task manager
    //write to stdout:
    char* username = getenv("USER");
    char hostname[TOKEN_SIZE];
    gethostname(hostname, TOKEN_SIZE);
    char tty[TOKEN_SIZE];
    ttyname_r(STDIN_FILENO, tty, TOKEN_SIZE);
    char* shell = getenv("SHELL");
    char* home = getenv("HOME");
    char* date = (char*) malloc(sizeof(char) * MAX_PATH_SIZE);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(date, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(stdout, "Username: %s\n", username);
    fprintf(stdout, "Hostname: %s\n", hostname);
    fprintf(stdout, "Last Executed Command: %s\n", lastExecuted);
    fprintf(stdout, "TTY: %s\n", tty);
    fprintf(stdout, "Current Shell Name: %s\n", shell);
    fprintf(stdout, "Home Location: %s\n", home);
    fprintf(stdout, "Current Time and Date: %s\n", date);
    fprintf(stdout, "Current number of processes being executed: %d\n", numberOfProcesses);
}

void fetchAlias(struct aliases* ali){
    FILE* fp = fopen(".aliases.txt", "r");
    if (fp == NULL){
        fp = fopen(".aliases.txt", "w");
        if (fp == NULL){
            printf("Error: Could not open nor create the configuration file.\n");
            return;
        }
        fclose(fp);
        return;
    }
    int i = 0;
    ali->size = 0;
    while (!feof(fp)){
        char* line = (char*) malloc(sizeof(char) * (2 * TOKEN_SIZE + 4));
        fgets(line, 2*TOKEN_SIZE +4, fp);
        if (strlen(line) == 0){
            continue;
        }
        char* var = strtok(line, "=");
        char* val = strtok(NULL, "=");
        if (var == NULL || val == NULL){
            continue;
        }
        if (val[strlen(val) - 1] == '\n'){
            val[strlen(val) - 1] = '\0';
        }
        strcpy(ali[i].key, var);
        strcpy(ali[i].value, val);
        i++;
    }
    ali->size = i;
    fclose(fp);
}

void addAlias(struct aliases* ali, char* var, char* val){
    ali->key[ali->size] = *var;
    ali->value[ali->size] = *val;
    ali->size++;
    FILE* fp = fopen(".aliases.txt", "a");
    if (fp == NULL){
        printf("The configuration file was corrupted or deleted during runtime.\n");
        return;
    }
    fprintf(fp, "%s=%s\n", var, val);
    fclose(fp);
}

char* searchAlias(struct aliases* ali, char* var){
    for (int i = 0; i < ali->size; i++){
        if (strcmp(ali[i].key, var) == 0){
            return ali[i].value;
        }
    }
    return var;
}

char* findPath(char* command){
    char* PATH = getenv("PATH");
    char* path = strtok(PATH, ":");
    while (path != NULL){
        char* commandPath = (char*) malloc(sizeof(char) * MAX_PATH_SIZE);
        strcpy(commandPath, path);
        strcat(commandPath, "/");
        strcat(commandPath, command);
        if (access(commandPath, X_OK) == 0){
            return commandPath;
        }
        path = strtok(NULL, ":");
    }
    return NULL;
}

void executeCommands(struct aliases* aliasList, struct token* operationList, struct Stack* argStack, int background, int redirection){



    struct token* operation = &operationList[CMDINDEX];
    if (strcmp(operation->value, "exit") == 0){
        exit(0);
    }
    if (strcmp(operation->value, "alias") == 0){
        //alias
        struct token* var = &operationList[ARGONE];
        struct token* val = &operationList[ARGTWO];
        addAlias(aliasList, var->value, val->value);
        return;
    }

    //Fork and execute:
    pid_t pid = fork();
    if (pid < 0){
        printf("Error: Could not fork.\n");
        exit(1);
    }
    if (pid == 0){
        switch (redirection){
            case(0) :{
                break;
            }
            case(1): {
                int fileDescriptor = open(operationList[REDIRECTIONFILEINDEX].value, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fileDescriptor == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fileDescriptor, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fileDescriptor);
                break;
            }
            case(2): {
                int fileDescriptor = open(operationList[REDIRECTIONFILEINDEX].value, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fileDescriptor == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fileDescriptor, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fileDescriptor);
                break;
            }
        }

        //child process
        //printf("Child process %d created.\n", getpid());
        if (strcmp(operation->value, "bello") == 0){
            belloFunc(operationList);
            exit(0);
        }
        else{
            //turn argstack into char* array:
            char* args_to_pass[MAX_TOKENS];
            args_to_pass[0] = operation->value;
            int i = 1;
            while (argStack->top != NULL){
                struct token* arg = (struct token*) pop(argStack);
                args_to_pass[i] = arg->value;
                i++;
            }
            args_to_pass[i] = NULL;

            //search path:
            char* absolutepath = findPath(operation->value);
            if (absolutepath == NULL){
                printf("Error: Command not found.\n");
                exit(1);
            }
            //printf("Executing in: %s\n", absolutepath);
            //printf("With arguments: %s\n", args_to_pass[0]);
            execv(absolutepath, args_to_pass);
            free(absolutepath);
            printf("Error: Command not found.\n");
            exit(1);
        }
    }
    else{
        //parent process
        strncpy(lastExecuted, operation->value, strlen(operation->value)+1);
        numberOfProcesses++;
        if (background == 1){
            //background process
            int status;
            waitpid(pid, &status, WNOHANG);
            if (status == 0){
                numberOfProcesses--;
            }
            else {
                printf("Error: Could not execute command.\n");
            }
        }
        else{
            //foreground process
            int status;
            waitpid(pid, &status, 0);

            if (status == 0){
                numberOfProcesses--;
            }
            else {
                printf("Error: Could not execute command.\n");
            }
        }
    }

}


void goTo(int state){
    i_push(stateStack, state);
}

void shift(int state, struct token *token){
    struct token *newtoken = token;
    push(tokenStack, newtoken);
    token = NULL;
    goTo(state);
}

void reduce(int rule, struct aliases* aliasList, struct token* operationList, struct Stack* argStack){
    switch (rule){
        case 0:{
            i_pop(stateStack);

            pop(tokenStack);
            struct token* token = (struct token*) pop(tokenStack);
            token->type = actual_CMD;
            push(tokenStack, token);
            break;
        }
        case 1:{
            //IMPLEMENT REDIRECTION WITH BACKGROUND PROCESS
            //fCMD -> CMD REDIRECT &
            i_pop(stateStack);

            struct token* amp = (struct token*) pop(tokenStack);
            struct token* redirect = (struct token*) pop(tokenStack);
            struct token* src = (struct token*) pop(tokenStack);
            if (strlen(src->value) + strlen(redirect->value) + strlen(amp->value) + 2 > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(src->value, " ");
            strncat(src->value, redirect->value, strlen(redirect->value));
            strcat(src->value, " ");
            strncat(src->value, amp->value, strlen(amp->value));
            src->type = fCMD;
            push(tokenStack, src);
            executeCommands(aliasList, operationList, argStack, 1,strlen(redirect->value));
            break;
        }
        case 2:{
            //IMPLEMENT BACKGROUND
            //fCMD -> CMD &
            i_pop(stateStack);

            struct token* amp = (struct token*) pop(tokenStack);
            struct token* cmd = (struct token*) pop(tokenStack);
            if (strlen(cmd->value) + strlen(amp->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(cmd->value, " ");
            strncat(cmd->value, amp->value, strlen(amp->value));
            cmd->type = fCMD;
            push(tokenStack, cmd);
            executeCommands(aliasList, operationList, argStack, 1, 0);
            break;
        }
        case 3:{
            //IMPLEMENT REDIRECTION
            //fCMD -> CMD REDIRECT
            i_pop(stateStack);
            struct token* redirect = (struct token*) pop(tokenStack);
            struct token* src = (struct token*) pop(tokenStack);
            if (strlen(src->value) + strlen(redirect->value) + 1 > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(src->value, " ");
            strncat(src->value, redirect->value, strlen(operationList[REDIRECTIONINDEX].value));
            src->type = fCMD;
            push(tokenStack, src);
            executeCommands(aliasList, operationList, argStack, 0, strlen(operationList[REDIRECTIONINDEX].value));
            break;
        }
        case 4:{
            //IMPLEMENT STRAIGHT COMMAND EXECUTION
            //fCMD -> CMD
            struct token* token = (struct token*) pop(tokenStack);
            token->type = fCMD;
            push(tokenStack, token);
            executeCommands(aliasList, operationList, argStack, 0, 0);
            break;
        }
        case 5:{
            //CMD -> VAR ARGS
            i_pop(stateStack);

            struct token* args = (struct token*) pop(tokenStack);
            struct token* var = (struct token*) pop(tokenStack);
            struct token* dup_var = (struct token*) malloc(sizeof(struct token) * TOKEN_SIZE);
            dup_var->type = var->type;
            strcpy(dup_var->value, var->value);
            operationList[CMDINDEX] = *dup_var;
            if (strlen(var->value) + strlen(args->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(var->value, " ");
            strncat(var->value, args->value, strlen(args->value));
            var->type = CMD;
            push(tokenStack, var);
            break;
        }
        case 6:{
            struct token* var = (struct token*) pop(tokenStack);
            struct token* dup_var = (struct token*) malloc(sizeof(struct token) * TOKEN_SIZE);
            dup_var->type = var->type;
            strcpy(dup_var->value, var->value);
            operationList[CMDINDEX] = *dup_var;
            var->type = CMD;
            push(tokenStack, var);
            break;
        }
        case 7:{
            //CMD -> bello
            struct token* token = (struct token*) pop(tokenStack);
            token->type = CMD;
            operationList[CMDINDEX] = *token;
            push(tokenStack, token);
            break;

        }
        case 8:{
            //CMD -> exit
            struct token* token = (struct token*) pop(tokenStack);
            token->type = CMD;
            operationList[CMDINDEX] = *token;
            push(tokenStack, token);
            break;
        }
        //foreground waitnull, background waitpit reaps the child process
        case 9:{
            //CMD -> alias EXT-WORD = EXT-WORD
            i_pop(stateStack);
            i_pop(stateStack);
            i_pop(stateStack);

            struct token* ext2 = (struct token*) pop(tokenStack);
            struct token* equal = (struct token*) pop(tokenStack);
            struct token* ext1 = (struct token*) pop(tokenStack);
            struct token* alias = (struct token*) pop(tokenStack);
            struct token* dup_alias = (struct token*) malloc(sizeof(struct token) * TOKEN_SIZE);
            alias->type = CMD;
            dup_alias->type = alias->type;
            strncpy(dup_alias->value, alias->value, strlen(alias->value));
            operationList[ARGONE] = *ext1;
            operationList[ARGTWO] = *ext2;
            operationList[CMDINDEX] = *alias;
            if (strlen(dup_alias->value) + strlen(ext1->value) + strlen(ext2->value) + strlen(equal->value) + 3 > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(dup_alias->value, " ");
            strncat(dup_alias->value, ext1->value, strlen(ext1->value));
            strcat(dup_alias->value, " ");
            strncat(dup_alias->value, equal->value, strlen(equal->value));
            strcat(dup_alias->value, " ");
            strncat(dup_alias->value, ext2->value, strlen(ext2->value));
            push(tokenStack, dup_alias);
            break;
        }
        case 10:
            //REDIRECT -> > EXT-WORD
        case 11:
            //REDIRECT -> >> EXT-WORD
        case 12:{
            //REDIRECT -> >>> EXT-WORD
            i_pop(stateStack);

            struct token* ext = (struct token*) pop(tokenStack);
            struct token* redirect = (struct token*) pop(tokenStack);
            if (strlen(redirect->value) + strlen(ext->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            //duplicate the tokens for convenience:
            struct token* dup_redirect = (struct token*) malloc(sizeof(struct token) * TOKEN_SIZE);
            dup_redirect->type = redirect->type;
            strcpy(dup_redirect->value, redirect->value);
            operationList[REDIRECTIONINDEX] = *dup_redirect;
            operationList[REDIRECTIONFILEINDEX] = *ext;
            strcat(redirect->value, " ");
            strncat(redirect->value, ext->value, strlen(ext->value));
            redirect->type = REDIRECT;
            push(tokenStack, redirect);
            break;
        }
        case 13:{
            //ARGS -> ARGS EXT-WORD
            i_pop(stateStack);

            struct token* ext = (struct token*) pop(tokenStack);
            struct token* args = (struct token*) pop(tokenStack);
            //add the two values and write it to args token and push it back on the stack, args->value is a char array:
            if (strlen(ext->value) + strlen(args->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            push(argStack, ext);
            strcat(args->value, " ");
            strncat(args->value, ext->value, strlen(ext->value));
            push(tokenStack, args);
            break;
        }
        case 14:{
            //ARGS -> EXT-WORD
            struct token* ext = (struct token*) pop(tokenStack);
            ext->type = ARGS;
            push(argStack, ext);
            push(tokenStack, ext);
            break;
        }
        case 15:{
            //VAR -> EXT-WORD
            struct token* ext = (struct token*) pop(tokenStack);
            ext->type = VAR;
            push(tokenStack, ext);
            break;
        }
        case 16:{
            //EXT-WORD -> " WORDS EXT-WORD "
            i_pop(stateStack);
            i_pop(stateStack);
            i_pop(stateStack);

            struct token* apost2 = (struct token*) pop(tokenStack);
            struct token* ext = (struct token*) pop(tokenStack);
            struct token* words = (struct token*) pop(tokenStack);
            struct token* apost1 = (struct token*) pop(tokenStack);
            if (strlen(apost1->value) + strlen(words->value) + 1 + strlen(ext->value) + strlen(apost2->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            char* testString = ext->value;
            while (*testString != '\0'){
                if (isspace(*testString)){
                    strncat(apost1->value, ext->value, strlen(ext->value));
                    strncat(apost1->value, apost2->value, strlen(apost2->value));
                    strncpy(ext->value, apost1->value, strlen(apost1->value));
                }
                testString++;
            }
            strncat(words->value, ext->value, strlen(ext->value));
            words->type = EXTWORD;
            push(tokenStack, words);
            break;
        }
        case 17:{
            //EXT-WORD -> " EXT-WORD "
            i_pop(stateStack);
            i_pop(stateStack);

            pop(tokenStack);
            struct token* ext = (struct token*) pop(tokenStack);
            pop(tokenStack);
            ext->type = EXTWORD;
            strncpy(ext->value, searchAlias(aliasList, ext->value), strlen(ext->value));
            push(tokenStack, ext);
            break;
        }
        case 18:{
            //EXT-WORD -> " WORDS "
            i_pop(stateStack);
            i_pop(stateStack);

            pop(tokenStack);
            struct token* ext = (struct token*) pop(tokenStack);
            pop(tokenStack);
            ext->type = EXTWORD;
            strncpy(ext->value, searchAlias(aliasList, ext->value), strlen(ext->value));
            push(tokenStack, ext);
            break;
        }
        case 19:{
            //EXT-WORD -> word
            struct token* word = (struct token*) pop(tokenStack);
            word->type = EXTWORD;
            strncpy(word->value, searchAlias(aliasList, word->value), strlen(word->value));
            push(tokenStack, word);
            break;
        }
        case 20:{
            //WORDS -> WORDS word
            i_pop(stateStack);

            struct token* word = (struct token*) pop(tokenStack);
            struct token* words = (struct token*) pop(tokenStack);
            if (strlen(words->value) + strlen(word->value) > TOKEN_SIZE){
                printf("Error: Argument too long.\n");
                break;
            }
            strcat(words->value, " ");
            strncat(words->value, word->value, strlen(word->value));
            push(tokenStack, words);
            break;
        }
        case 21:{
            //WORDS -> word
            struct token* word = (struct token*) pop(tokenStack);
            word->type = WORDS;
            push(tokenStack, word);
            break;
        }
        default: {
            printf("Error: Invalid rule.\n");
            break;
        }
    }
}

void displayMessage(){
    char* username = getenv("USER");
    char hostnameString[TOKEN_SIZE];
    char* cwd = getenv("PWD");
    //Check if the system calls were successful:
    if (username == NULL || cwd == NULL || gethostname(hostnameString, TOKEN_SIZE) == -1){
        printf("Error: Could not get the system information.\n");
        return;
    }
    char* display = (char*) calloc(sizeof(char), (strlen(username) + strlen(hostnameString) + strlen(cwd) + 8));
    strncpy(display, username, strlen(username));
    strcat(display, "@");
    strncat(display, hostnameString, strlen(hostnameString));
    strcat(display, " ");
    strncat(display, cwd, strlen(cwd));
    strcat(display, " --- ");
    fprintf(stdout,"%s", display);
}

int main(){
    signal(SIGCHLD, SIG_IGN);
    struct aliases* aliasList = calloc(2, (sizeof(char)*TOKEN_SIZE) * ALIAS_SIZE);
    fetchAlias(aliasList);
    compileregex();
    displayMessage();
    //start reading input
    while (fgets(msgbuf, sizeof(msgbuf), stdin)) {
        struct token* tokens = calloc(MAX_TOKENS, sizeof(struct token));
        struct token* operationList = calloc(10, sizeof(struct token));
        struct Stack* argStack = calloc(MAX_TOKENS, sizeof(struct Stack));

        //Tokenize the input:
        lexer(tokens);

        //Parse the input:
        i_init(&stateStack);
        init(&tokenStack);
        i_push(stateStack, 0);
        int condition = 1;
        int reduced = 0;
        int step = 0;

        while (condition) {
            if ((step == 0) && (tokens[0].type == eol)){
                break;
            }
            if (tokenIndex == MAX_TOKENS-1) {
                printf("Error: Too many tokens.\n");
                break;
            }

            struct token *nextToken;
            if (reduced) {
                nextToken = ((struct token *) peek(tokenStack));
            } else {
                nextToken = &tokens[tokenIndex];
            }

            int type = (*nextToken).type;
            int currentState = i_pop(stateStack);
            //printf("current state, type: %d, %d\n", currentState, type);
            //printf("parsingTable: {%d,%d}\n", parsingTable[currentState][type][0], parsingTable[currentState][type][1]);
            int action = parsingTable[currentState][type][0];
            int targetState = parsingTable[currentState][type][1];

            reduced = 0;
            step++;
            switch (action) {
                //accept the statement
                case -1: {
                    //ERROR CASE
                    printf("Error: Invalid input.\n");
                    condition = 0;
                    break;
                }
                case 0:
                    //SHIFT CASE
                    i_push(stateStack, currentState);
                    shift(targetState, nextToken);
                    //printf("s%d\n", targetState);
                    nextToken = NULL;
                    tokenIndex++;
                    break;
                case 1:
                    //REDUCE CASE
                    reduce(targetState, aliasList, operationList, argStack);
                    //printf("r%d\n", targetState);
                    //printf("popped state: %d\n", currentState);
                    //printf("current state: %d\n", i_peek(stateStack));
                    reduced = 1;
                    break;
                case 2:
                    //GOTO CASE
                    i_push(stateStack, currentState);
                    //printf("%d\n", targetState);
                    goTo(targetState);
                    break;
                case 3:
                    //ACCEPT CASE
                    //printf("Statement accepted\n");
                    condition = 0;
                    break;
                default:{
                    printf("%s\n","Something has gone really, really wrong.");
                    break;
                }
            }
        }
        free(stateStack);
        free(tokenStack);

        free(operationList);
        free(tokens);
        tokenIndex = 0;
        displayMessage();
    }
    regfree(&regex);
}