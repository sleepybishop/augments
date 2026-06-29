OBJ=interval_tree.o


CFLAGS   = -O2 -g -std=c11 -Wall -I.
CFLAGS  += -march=native -funroll-loops -ftree-vectorize -Wno-unused

all: $(OBJ);

main: main.o $(OBJ)

clean:
	$(RM) *.o

indent:
	find -name '*.[h,c]' | xargs clang-format -i

gperf: LDLIBS = -lprofiler -ltcmalloc
gperf: clean t/00util/bench
	CPUPROFILE_FREQUENCY=100000000 CPUPROFILE=gperf.prof cat data_sample.txt | ./main
	pprof ./main gperf.prof --callgrind > callgrind.gperf
	gprof2dot --format=callgrind callgrind.gperf -z main | dot -T svg > gperf.svg

ubsan: CC=clang
ubsan: CFLAGS += -fsanitize=undefined,implicit-conversion
ubsan: LDLIBS += -lubsan
ubsan: clean main
	cat data_sample.txt | ./main
										
