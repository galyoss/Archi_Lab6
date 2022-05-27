all: exec

exec: task3.c LineParser.c
	gcc -g -m32 -c -o LineParser.o LineParser.c
	gcc -g -m32 -c -o task3.o task3.c
	gcc -g -m32 task3.o LineParser.o -o main
	rm *.o

.PHONY: clean
clean:
	rm -rf ./*.o main