//
// Created by ultiminati on 10/12/2023.
//

#ifndef CSELLSCSHELLS_STACKS_H
#define CSELLSCSHELLS_STACKS_H


struct Node{
    struct token *data;
    struct Node *next;
    int state;
};

struct Stack{
    struct Node *top;
    int size;
};


struct intNode{
    int data;
    struct intNode *next;
};

struct intStack{
    struct intNode *top;
    int size;
};


void init(struct Stack **stack);
void push(struct Stack *stack, struct token *data);
struct token *pop(struct Stack *stack);
struct token *peek(struct Stack *Stack);

void i_init(struct intStack **Stack);
void i_push(struct intStack *stack, int data);
int i_pop(struct intStack *Stack);
int i_peek(struct intStack *Stack);


#endif //CSELLSCSHELLS_STACKS_H
