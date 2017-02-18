.build/test.o: test/test.c src/dobject.h src/dobject.c
	mkdir -p .build
	$(CC) -Wall -Os test/test.c src/dobject.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

run: .build/test.o
	@if .build/test.o ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

