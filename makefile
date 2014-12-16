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
	# time ./bin/sc -v ./tst/RightsOfManA.png ./tst/out.png
	# time ./bin/sc -v ./tst/BPD-01.png ./tst/out_BPD-01.png
	# time ./bin/sc -v ./tst/BPD-01b.png ./tst/out_BPD-01b.png
	# time ./bin/sc -v ./tst/BPD-01c.png ./tst/out_BPD-01c.png
	# time ./bin/sc -v ./tst/BPD-02.png ./tst/out_BPD-02.png
	# time ./bin/sc -v ./tst/BPD-03.png ./tst/out_BPD-03.png
	# time ./bin/sc -v ./tst/BPD-04.png ./tst/out_BPD-04.png
	time ./bin/sc -v ./tst/Broadway_tower_edit.png ./tst/out_Broadway.png
	#
	# time ./bin/sc -v ./tst/BPD-01c-p1.png ./tst/out_BPD-01c-p1.png
	# time ./bin/sc -v ./tst/BPD-01c-p2.png ./tst/out_BPD-01c-p2.png
	# time ./bin/sc -v ./tst/BPD-01c-p3.png ./tst/out_BPD-01c-p3.png
	# time ./bin/sc -v ./tst/BPD-01c-p4.png ./tst/out_BPD-01c-p4.png
	# time ./bin/sc -v ./tst/BPD-01c-p5.png ./tst/out_BPD-01c-p5.png
	# time ./bin/sc -v ./tst/BPD-01c-p6.png ./tst/out_BPD-01c-p6.png
	# time ./bin/sc -v ./tst/BPD-01c-p7.png ./tst/out_BPD-01c-p7.png
	# time ./bin/sc -v ./tst/BPD-01c-p8.png ./tst/out_BPD-01c-p8.png
	# 
#	time ./bin/sc -v ./tst/Breuel-a.png ./tst/out_Breuel-a.png
#	time ./bin/sc -v ./tst/Breuel-ba.png ./tst/out_Breuel-ba.png
#	time ./bin/sc -v ./tst/Breuel-bb.png ./tst/out_Breuel-bb.png
#	time ./bin/sc -v ./tst/Breuel-bc.png ./tst/out_Breuel-bc.png
	# time ./bin/sc -v ./tst/cmp.png ./tst/out_cmp.png
	time ./bin/sc -v ./tst/002.png ./tst/out_002.png


clean:
	-rm ./bin/*
