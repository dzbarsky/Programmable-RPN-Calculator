all: pcalc

pcalc: pcalc.c
	clang -g -o pcalc pcalc.c

clean:
	rm -rf pcalc
