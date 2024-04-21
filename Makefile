build:
	gcc \
	-Wall \
	-std=c99 \
	-lSDL2 \
	-lm \
	-lc \
	-O3 \
	src/*.c -o renderer

run:
	./renderer

clean:
	rm ./renderer