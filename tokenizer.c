//
// Created by ultiminati on 10/12/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "tokenizer.h"
const int numberofgroups = 7;

extern regex_t regex;
regmatch_t match[8];
int regexVal;
extern char msgbuf[257];


const char* pattern = "([\"\'])|(>>>)|(>>)|(>)|(=)|(&)|([^\"\'>=&[:space:]]+)";

// Using regular expression: (["'])|(>>>)|(>>)|(>)|(=)|(&)|([^"'>=&[:space:]]+) to tokenize the input
//

enum type getType(char *string, int group){
    switch(group){
    case 0:
        return apostrophe;
    case 1:
        return redir3;
    case 2:
        return redir2;
    case 3:
        return redir;
    case 4:
        return equals;
    case 5:
        return ampersand;
    case 6:
        if (strcmp(string, "") == 0) return eol;
        else if (strcmp(string, "bello") == 0) return bello;
        else if (strcmp(string, "exit") == 0) return exittoken;
        else if (strcmp(string, "alias") == 0) return alias;
        else return word;

    default:
        return word;
    }

}


struct token *tokenize(char *string, int len, int group){
    enum type type = getType(string, group);
    struct token *token = malloc(sizeof(struct token) + (len+1)*sizeof(char));
    token->type = type;
    strncpy(token->value, string, len+1);
    return token;
}




int compileregex(){
    //compile the regex pattern
    regexVal = regcomp(&regex, pattern, REG_EXTENDED);
    if (regexVal) {
        fprintf(stderr, "Regex compilation error\n");
        return 1;
    }
    return 0;
}

void lexer(struct token* tokens) {
    // execute search
    regexVal = regexec(&regex, msgbuf, numberofgroups+1, match, 0);  // Assuming 7 capturing groups + entire match
    int lastPtr = 0;
    int i = 0;

    // look for all matches in the line
    while (regexVal == 0) {
        for (int group = 0; group < numberofgroups; ++group) { // Iterate through all groups
            if (match[group + 1].rm_eo != -1) {  // Check if the group is matched
                int startBuffer = match[group + 1].rm_so;
                int endBuffer = match[group + 1].rm_eo;
                int len = endBuffer - startBuffer;

                // copy the match to a string
                char tokenStr[len + 1];
                strncpy(tokenStr, msgbuf + lastPtr + startBuffer, len);
                tokenStr[len] = '\0';

                // tokenize and add to array
                if (!isspace((int) tokenStr[0])) {
                    struct token *token = tokenize(tokenStr, len, group);
                    tokens[i] = *token;
                    i++;
                }
            }
        }
        // go to the next match
        lastPtr += match[0].rm_eo;
        regexVal = regexec(&regex, msgbuf + lastPtr, numberofgroups+1, match, 0);
    }
    struct token* eoltoken = tokenize("", 0, 6);
    tokens[i] = *eoltoken;
}

//int main() {
//    msgbuf[0] = '\0';
//    // Initialize the regex pattern
//    if (compileregex() != 0) {
//        return 1;
//    }
//
//    // Buffer to store user input
//    char inputBuffer[257];
//
//    // Tokens array to store the result
//    struct token tokens[100];  // Adjust the size as needed
//
//    // Prompt the user for input
//    printf("Enter a string to tokenize (Ctrl+D or Ctrl+Z to end input):\n");
//
//    // Read input until EOF (Ctrl+D or Ctrl+Z)
//    while (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
//        // Ensure the inputBuffer is null-terminated
//        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
//
//        // Reset the msgbuf for each input
//        printf("Input: %s\n", inputBuffer);
//        printf("msgbuf: %s\n", msgbuf);
//        strcpy(msgbuf, inputBuffer)//        printf("msgbuf: %s\n", msgbuf);//        // Tokenize the input//        lexer(tokens);
//
//        // Print the tokens
//        for (int i = 0; i< 100; ++i) {
//            printf("Token Type: %d, Value: %s\n", tokens[i].type, tokens[i].value);
//            if (tokens[i].type == eol) break;
//        }
//
//        // Prompt the user for the next input
//        printf("\nEnter a string to tokenize (Ctrl+D or Ctrl+Z to end input):\n");
//    }
//
//    // Free the compiled regex
//    regfree(&regex);
//
//    return 0;
//}


