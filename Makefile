all: test_ng

clean:
	rm -rf ng *.o

test_ng: main.o
	clang $^ -o $@
main.o: main.c *.h
	clang -Werror -ferror-limit=1 -std=c11 -pedantic-errors \
	-O0 -ggdb \
	-I. \
	-c $< -o $@
