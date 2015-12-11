#
# sc3d -- Experiments in seam carving over three dimensions
#  Copyright (c) 2015 Christopher Stoll (https://github.com/stollcri)
#

CC = cc

CFLAGS = -O3 -Wall -D PNG16BIT
IFLAGS = -I /usr/local/include/libpng16
LFLAGS = -l ncurses -l teem -l png -l bz2 -l z -l m
AOFILE = ./bin/sc3d
CFILES = src/sc3d.c src/libColorConv.c src/libEnergies3D.c src/libMinMax.c src/libpngHelper.c

CFLAGS_DBG = -O0 -g -Weverything -D PNG16BIT
CFLAGS_DBG = -O0 -g -D PNG16BIT
# CFLAGS_DBG = -O0 -g
IFLAGS_DBG = $(IFLAGS)
LFLAGS_DBG = $(LFLAGS)
AOFILE_DBG = $(AOFILE)
CFILES_DBG = $(CFILES)

default: test
test: testbuild run

testbuild:
	$(CC) $(CFLAGS_DBG) $(LFLAGS_DBG) -o $(AOFILE_DBG) $(CFILES_DBG)

build:
	$(CC) $(CFLAGS) $(LFLAGS) -o $(AOFILE) $(CFILES)

run:
	$(AOFILE_DBG) tst/sc3d/chest.vtk out/headsq/sc3d.vtk
	# $(AOFILE_DBG) tst/sc3d/head.vtk out/headsq/sc3d.vtk
	# $(AOFILE_DBG) tst/sc3d/headsq/quarter.nhdr out/headsq/sc3d.vtk

.PHONY: default test testbuild build run all sc test-sc test-sc-all clean-sc

#
# sc -- Experiments in seam carving for text line extraction
#  Copyright (c) 2015 Christopher Stoll (https://github.com/stollcri)
#

CFLAGS_SC = -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16
CFLAGS_SC_FULL = -Weverything -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16
CFLAGS_SC_SYNTAX = -fsyntax-only -Weverything -I/usr/local/include/libpng16 -L/usr/local/lib -lpng16

scall: sc test-sc

sc:
	# mkdir -p ./bin/
	${CC} ${CFLAGS_SC} -o ./bin/sc ./src/sc.c

test-sc:
	./bin/sc -b 8 -c 2 -d 4 -e 0 -g 0 tst/sc/Fig2a.png out/sc.png
	# ./bin/sc -b 9 -c 0 -d a -e 4 -g 0 ./tst/sc/BPD-01c.png ./tst/sc/out.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/sc/Broadway_tower_edit_small.png ./tst/sc/out-00.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/sc/Fig2a.png ./tst/sc/out-01.png
	# ./bin/sc -b 9 -c 0 -d c -e 4 -g 0 ./tst/sc/Fig2b.png ./tst/sc/out-02.png

test-sc-all:
	# ./bin/sc -b 0 -c 2 -d a -e 0 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01.png
	# ./bin/sc -b 0 -c 2 -d b -e 0 -g 0 ./tst/sc/BPD-01d-01.png ./tst/sc/out_BPD-02.png
	# ./bin/sc -b 0 -c 2 -d b -e 0 -g 0 ./tst/sc/BPD-01d-02.png ./tst/sc/out_BPD-03.png

	# ./bin/sc -b 0 -c 2 -d 5 -e 0 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01-Energy-0_DoG.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 1 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01-Energy-1_LoG.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 4 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01-Energy-4_Sobel.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 6 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01-Energy-6_Grad.png
	# ./bin/sc -b 0 -c 2 -d 5 -e 7 -g 0 ./tst/sc/BPD-01d.png ./tst/sc/out_BPD-01-Energy-7_DoG-Sobel.png

	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-352degree.png ./tst/sc/out_ROMB-352.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-356degree.png ./tst/sc/out_ROMB-356.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-357degree.png ./tst/sc/out_ROMB-357.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/sc/RightsOfManB-358degree.png ./tst/sc/out_ROMB-358.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-359degree.png ./tst/sc/out_ROMB-359.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/sc/RightsOfManA.png ./tst/sc/out_ROMB-000.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-001degree.png ./tst/sc/out_ROMB-001.png
	# ./bin/sc -b 0 -c 0 -d 9 -e 0 ./tst/sc/RightsOfManB-002degree.png ./tst/sc/out_ROMB-002.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-003degree.png ./tst/sc/out_ROMB-003.png
	# ./bin/sc -b 0 -c 0 -d 7 -e 0 ./tst/sc/RightsOfManB-004degree.png ./tst/sc/out_ROMB-004.png
	# ./bin/sc -b 0 -c 0 -d 8 -e 0 ./tst/sc/RightsOfManB-008degree.png ./tst/sc/out_ROMB-008.png

	# time ./bin/sc -b 0 -c 0 -d 0 -e 0 ./tst/sc/Broadway_tower_edit.png ./tst/sc/out_Broadway-0.png
	# time ./bin/sc -b 0 -c 0 -d 1 -e 0 ./tst/sc/Broadway_tower_edit.png ./tst/sc/out_Broadway-1.png
	# time ./bin/sc -b 0 -c 0 -d 2 -e 0 ./tst/sc/Broadway_tower_edit.png ./tst/sc/out_Broadway-2.png
	# time ./bin/sc -b 0 -c 0 -d 3 -e 0 ./tst/sc/Broadway_tower_edit.png ./tst/sc/out_Broadway-3.png
	# time ./bin/sc -b 0 -c 0 -d 0 -e 0 ./tst/sc/Broadway_tower_edit.png ./tst/sc/out_Broadway-4.png

	# time ./bin/sc      -d 0 -e 0 ./tst/sc/BPD-01c.png ./tst/sc/out_BPD-01c-0.png
	# time ./bin/sc -b 0 -c 0 -d 1 -e 0 ./tst/sc/BPD-01c.png ./tst/sc/out_BPD-01c-1.png
	# time ./bin/sc -b 0 -c 0 -d 2 -e 0 ./tst/sc/BPD-01c.png ./tst/sc/out_BPD-01c-2.png
	# time ./bin/sc -b 0 -c 3 -d 3 -e 0 ./tst/sc/BPD-01c.png ./tst/sc/out_BPD-01c-3.png

	# time ./bin/sc -b 0 -c 3 -d 3 -e 8 -g 0 ./tst/sc/Fig2a.png ./tst/sc/Fig2a-out.png
	# time ./bin/sc -b 0 -c 3 -d 3 -e 8 -g 0 ./tst/sc/Fig2b.png ./tst/sc/Fig2b-out.png
	# For Protein Gel Analysis: perhaps divide the image like a kD-tree

clean-sc:
	-rm ./bin/*
