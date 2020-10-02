all:
	mkdir bin -p
	gcc src/main.c -o bin/main -Wall -Werror -lm -fsanitize=address,leak
clean:
	rm bin/main
	rmdir bin
