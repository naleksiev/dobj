.build/test.o: test/test.c src/dobj.h src/dobj.c
	mkdir -p .build
	$(CC) -std=c99 -Wall -Os test/test.c src/dobj.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

run: .build/test.o
	@if .build/test.o ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

