all: datareader.o filewriter.o aux.o
	gcc -o main datareader.o filewriter.o aux.o -lm

datareader.o: datareader.c datareader.h
	gcc -c datareader.c

filewriter.o: filewriter.c filewriter.h
	gcc -c filewriter.c

aux.o: aux.c aux.h
	gcc -c aux.c

run: all
	./main

clean:
	rm *.o *~
