.PHONY = all clean

CC = clang++

all:
	${CC} bwtsearch.cpp -o bwtsearch

clean:
	rm -rf *.o
