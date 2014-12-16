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
	#time ./bin/sc -v -d 1 ./tst/002.png ./tst/out_002-1.png
	time ./bin/sc -v      ./tst/Broadway_tower_edit.png ./tst/out_Broadway-0.png
	# time ./bin/sc -v -d 1 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-1.png
	# time ./bin/sc -v -d 2 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-2.png
	time ./bin/sc -v      ./tst/BPD-01c.png ./tst/out_BPD-01c-0.png
	# time ./bin/sc -v -d 1 ./tst/BPD-01c.png ./tst/out_BPD-01c-1.png
	# time ./bin/sc -v -d 2 ./tst/BPD-01c.png ./tst/out_BPD-01c-2.png

clean:
	-rm ./bin/*
