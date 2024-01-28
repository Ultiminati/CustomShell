#include <stdlib.h>
#include "stacks.h"


void init(struct Stack **stack){
    struct Stack *newStack = (struct Stack *)malloc(sizeof(struct Stack));
    newStack->size = 0;
    newStack->top = NULL;

    *stack = newStack;

}

void push(struct Stack *stack, struct token *data){
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = (stack->top);
    stack->top = newNode;
}

struct token *pop(struct Stack *stack){
    struct Node *temp = (stack->top);
    stack->top = ((stack->top)->next);
    struct token *data = temp->data;
    free(temp);
    return data;
}


struct token *peek(struct Stack *stack){
    return (stack->top)->data;
}






void i_init(struct intStack **stack){
    struct intStack *newStack = (struct intStack *)malloc(sizeof(struct intStack));
    newStack->size = 0;
    newStack->top = NULL;

    *stack = newStack;
}

void i_push(struct intStack *stack, int data){
    struct intNode *newNode = (struct intNode *)malloc(sizeof(struct intNode));
    newNode->data = data;
    struct intNode *temp = stack->top;
    newNode->next = temp;
    stack->top = newNode;
}

int i_pop(struct intStack *stack){
    struct intNode *temp = stack->top;
    stack->top = ((stack->top)->next);
    int data = temp->data;
    free(temp);
    return data;
}

int i_peek(struct intStack *stack){
    return (stack->top)->data;
}

