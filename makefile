myshell: main.o tokenizer.o stacks.o
	gcc main.o tokenizer.o stacks.o -o myshell

main.o: main.c tokenizer.h stacks.h
	gcc -g -c main.c

tokenizer.o: tokenizer.c tokenizer.h
	gcc -g -c tokenizer.c

stacks.o: stacks.c stacks.h
	gcc -g -c stacks.c
