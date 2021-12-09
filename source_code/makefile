CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -g3

all: mrc_2d_curve_weaken mrc_2d_curve_weaken_dynamic_mask mrc_2d_curve_weaken_dynamic_mask_display
mrc_2d_curve_weaken: driver_main.o mrc.o
	${CC} ${CCFLAGS} -o $@ $^ -lm
mrc_2d_curve_weaken_dynamic_mask: driver_dm_main.o mrc.o
	${CC} ${CCFLAGS} -o $@ $^ -lm
mrc_2d_curve_weaken_dynamic_mask_display: driver_dm_display_main.o mrc.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

driver_main.o:mrc.h
driver_dm_main.o:mrc.h
driver_dm_display_main.o:mrc.h

mrc.o:mrc.h
clean:
	rm *o 
