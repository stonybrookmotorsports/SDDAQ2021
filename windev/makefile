all: datareader.o filewriter.o auxf.o
	gcc -o main datareader.o filewriter.o auxf.o -lm

datareader.o: datareader.c datareader.h
	gcc -c datareader.c

filewriter.o: filewriter.c filewriter.h
	gcc -c filewriter.c

aux.o: auxf.c auxf.h
	gcc -c auxf.c

run: all
	./main

clean:
	rm *.o *~
