build:
	gcc -Wall -std=c99 -Winline \
	-lSDL2 \
	-lm \
	src/*.c -o renderer

run:
	./renderer

clean:
	rm ./renderer