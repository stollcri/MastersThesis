CC = cc
CFLAGS = -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16
CFLAGS_FULL = -Weverything -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16

default: all

.PHONY: all sc clean
all: sc test

sc:
	mkdir -p ./bin/
	${CC} ${CFLAGS} -o ./bin/sc ./src/sc.c

test:
	./bin/sc ./tst/RightsOfManB.png

clean:
	-rm ./bin/*
