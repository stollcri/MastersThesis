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
	./bin/sc -b 8 -c 2 -d 4 -e 0 -g 0 ./tst/1.png ./tst/out.png
	# ./bin/sc -b 9 -c 0 -d a -e 4 -g 0 ./tst/BPD-01c.png ./tst/out.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/Broadway_tower_edit_small.png ./tst/out-00.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/Fig2a.png ./tst/out-01.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/Fig2b.png ./tst/out-02.png

test-all:
	# ./bin/sc -b 0 -c 2 -d a -e 0 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01.png
	# ./bin/sc -b 0 -c 2 -d b -e 0 -g 0 ./tst/BPD-01d-01.png ./tst/out_BPD-02.png
	# ./bin/sc -b 0 -c 2 -d b -e 0 -g 0 ./tst/BPD-01d-02.png ./tst/out_BPD-03.png
	
	# ./bin/sc -b 0 -c 2 -d 5 -e 0 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01-Energy-0_DoG.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 1 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01-Energy-1_LoG.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 4 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01-Energy-4_Sobel.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 6 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01-Energy-6_Grad.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 7 -g 0 ./tst/BPD-01d.png ./tst/out_BPD-01-Energy-7_DoG-Sobel.png
	
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-352degree.png ./tst/out_ROMB-352.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-356degree.png ./tst/out_ROMB-356.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-357degree.png ./tst/out_ROMB-357.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/RightsOfManB-358degree.png ./tst/out_ROMB-358.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-359degree.png ./tst/out_ROMB-359.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/RightsOfManA.png ./tst/out_ROMB-000.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-001degree.png ./tst/out_ROMB-001.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/RightsOfManB-002degree.png ./tst/out_ROMB-002.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-003degree.png ./tst/out_ROMB-003.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/RightsOfManB-004degree.png ./tst/out_ROMB-004.png
	# ./bin/sc -b 0 -c 0 -d 8 -e 0 ./tst/RightsOfManB-008degree.png ./tst/out_ROMB-008.png

	# time ./bin/sc -b 0 -c 0 -d 0 -e 0 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-0.png
	# time ./bin/sc -b 0 -c 0 -d 1 -e 0 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-1.png
	# time ./bin/sc -b 0 -c 0 -d 2 -e 0 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-2.png
	# time ./bin/sc -b 0 -c 0 -d 3 -e 0 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-3.png
	# time ./bin/sc -b 0 -c 0 -d 0 -e 0 ./tst/Broadway_tower_edit.png ./tst/out_Broadway-4.png
	
	# time ./bin/sc      -d 0 -e 0 ./tst/BPD-01c.png ./tst/out_BPD-01c-0.png
	# time ./bin/sc -b 0 -c 0 -d 1 -e 0 ./tst/BPD-01c.png ./tst/out_BPD-01c-1.png
	# time ./bin/sc -b 0 -c 0 -d 2 -e 0 ./tst/BPD-01c.png ./tst/out_BPD-01c-2.png
	# time ./bin/sc -b 0 -c 3 -d 3 -e 0 ./tst/BPD-01c.png ./tst/out_BPD-01c-3.png

	# time ./bin/sc -b 0 -c 3 -d 3 -e 8 -g 0 ./tst/Fig2a.png ./tst/Fig2a-out.png
	# time ./bin/sc -b 0 -c 3 -d 3 -e 8 -g 0 ./tst/Fig2b.png ./tst/Fig2b-out.png
	# For Protein Gel Analysis: perhaps divide the image like a kD-tree

clean:
	-rm ./bin/*
