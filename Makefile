build:
	gcc -Wall -std=c99 -Winline \
	-lSDL2 \
	-lm \
	-lc \
	src/*.c -o renderer

run:
	./renderer

clean:
	rm ./renderer