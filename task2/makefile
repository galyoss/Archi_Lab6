all: exec

exec: task2.c LineParser.c
	gcc -g -m32 -c -o LineParser.o LineParser.c
	gcc -g -m32 -c -o task2.o task2.c
	gcc -g -m32 task2.o LineParser.o -o main
	rm *.o

.PHONY: clean
clean:
	rm -rf ./*.o main