all: datareader.o filewriter.o
	gcc -o main datareader.o filewriter.o  -lm

reader.o: datareader.c datareader.h
	gcc -c datareader.c

filewriter.o: filewriter.c filewriter.h
	gcc -c filewriter.c

run: all
	./main

clean:
	rm *.o *~
