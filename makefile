CC = cc
CFLAGS = -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16
CFLAGS_FULL = -Weverything -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16
CFLAGS_SYNTAX = -fsyntax-only -Weverything -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16

default: all

.PHONY: all sc clean
all: sc test

sc:
	mkdir -p ./bin/
	${CC} ${CFLAGS} -o ./bin/sc ./src/sc.c

test:
	#time ./bin/sc -v ./tst/RightsOfManA.png ./tst/out.png
	time ./bin/sc -d 1 -v ./tst/BPD-01.png ./tst/out_BPD-01.png
	time ./bin/sc -d 2 -v ./tst/BPD-01b.png ./tst/out_BPD-01b.png
	#time ./bin/sc -v ./tst/BPD-02.png ./tst/out_BPD-02.png
	#time ./bin/sc -v ./tst/BPD-03.png ./tst/out_BPD-03.png
	#time ./bin/sc -v ./tst/BPD-04.png ./tst/out_BPD-04.png

clean:
	-rm ./bin/*
