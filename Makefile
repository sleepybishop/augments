OBJ=src/interval_tree.o


CFLAGS   = -O2 -g -std=c11 -Wall -I.
CFLAGS  += -march=native -funroll-loops -ftree-vectorize -Wno-unused

all: $(OBJ);

main: main.o $(OBJ)

images: t/util/test_helper
	mkdir -p images
	(echo "ITREE ADD 1|10|20"; echo "ITREE ADD 2|15|25"; echo "ITREE ADD 3|5|15"; echo "ITREE ADD 4|30|40"; echo "ITREE ADD 5|25|35"; echo "ITREE ADD 6|8|12"; echo "ITREE ADD 7|1|6"; echo "ITREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/interval_tree.png

clean:
	$(RM) src/*.o t/util/*.o *.o main test_helper

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
										
