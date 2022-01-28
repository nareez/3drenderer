ifeq ($(shell uname -s),Linux)
	EXEC := gcc -Wall -std=c99 ./src/*.c -lSDL2 -o renderer			
else
	EXEC := clang -Wall -std=c99 ./src/*.c -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -o renderer
endif

build:
	$(EXEC)

run:
	./renderer

clean:
	rm renderer