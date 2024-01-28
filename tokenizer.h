//
// Created by ultiminati on 10/12/2023.
//

#ifndef CSELLSCSHELLS_TOKENIZER_H
#define CSELLSCSHELLS_TOKENIZER_H


#define TOKEN_SIZE 256

//enum corresponding to non-terminals and terminals
//declared within the grammar of the language.
enum type{
    eol,
    ampersand,
    bello,
    exittoken,
    alias,
    equals,
    redir,
    redir2,
    redir3,
    apostrophe,
    word,
    fCMD,
    CMD,
    REDIRECT,
    ARGS,
    VAR,
    EXTWORD,
    WORDS,
    actual_CMD,
};

struct token{
    enum type type;
    char value[TOKEN_SIZE+1];
};


enum type getType(char *string, int group);
struct token *tokenize(char *string, int len, int group);
int compileregex();
void lexer(struct token* tokens);


#endif //CSELLSCSHELLS_TOKENIZER_H
