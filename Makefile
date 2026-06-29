OBJ=src/interval_tree.o \
    src/max_subarray.o \
    src/order_statistic.o \
    src/priority_search.o \
    src/range_min.o \
    src/range_sum.o \
    src/hash_tree.o \
    src/lcp_tree.o


CFLAGS   = -O2 -g -std=c11 -Wall -I.
CFLAGS  += -march=native -funroll-loops -ftree-vectorize -Wno-unused

all: $(OBJ);

main: main.o $(OBJ)

t/util/test_helper: t/util/test_helper.o $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

check: t/util/test_helper
	prove -v t/

images: t/util/test_helper
	mkdir -p images
	(echo "ITREE ADD 1|10|20"; echo "ITREE ADD 2|15|25"; echo "ITREE ADD 3|5|15"; echo "ITREE ADD 4|30|40"; echo "ITREE ADD 5|25|35"; echo "ITREE ADD 6|8|12"; echo "ITREE ADD 7|1|6"; echo "ITREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/interval_tree.png
	(echo "MAXSUB ADD 1|3.0"; echo "MAXSUB ADD 2|-2.0"; echo "MAXSUB ADD 3|5.0"; echo "MAXSUB ADD 4|-1.0"; echo "MAXSUB ADD 5|4.0"; echo "MAXSUB ADD 6|-3.0"; echo "MAXSUB ADD 7|2.0"; echo "MAXSUB GRAPH") | ./t/util/test_helper | dot -Tpng -o images/max_subarray.png
	(echo "OSTREE ADD 10"; echo "OSTREE ADD 20"; echo "OSTREE ADD 30"; echo "OSTREE ADD 40"; echo "OSTREE ADD 50"; echo "OSTREE ADD 60"; echo "OSTREE ADD 70"; echo "OSTREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/order_statistic.png
	(echo "PSTREE ADD 10|20"; echo "PSTREE ADD 15|25"; echo "PSTREE ADD 5|15"; echo "PSTREE ADD 25|30"; echo "PSTREE ADD 20|10"; echo "PSTREE ADD 8|12"; echo "PSTREE ADD 30|5"; echo "PSTREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/priority_search.png
	(echo "MINTREE ADD 1|10.0"; echo "MINTREE ADD 2|5.0"; echo "MINTREE ADD 3|15.0"; echo "MINTREE ADD 4|3.0"; echo "MINTREE ADD 5|8.0"; echo "MINTREE ADD 6|12.0"; echo "MINTREE ADD 7|1.0"; echo "MINTREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/range_min.png
	(echo "SUMTREE ADD 1|10.0"; echo "SUMTREE ADD 2|5.0"; echo "SUMTREE ADD 3|15.0"; echo "SUMTREE ADD 4|3.0"; echo "SUMTREE ADD 5|8.0"; echo "SUMTREE ADD 6|12.0"; echo "SUMTREE ADD 7|1.0"; echo "SUMTREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/range_sum.png
	(echo "HASHTREE ADD 1|a"; echo "HASHTREE ADD 2|b"; echo "HASHTREE ADD 3|c"; echo "HASHTREE ADD 4|d"; echo "HASHTREE ADD 5|e"; echo "HASHTREE ADD 6|f"; echo "HASHTREE ADD 7|g"; echo "HASHTREE GRAPH") | ./t/util/test_helper | dot -Tpng -o images/hash_tree.png
	(echo "LCP ADD apple"; echo "LCP ADD application"; echo "LCP ADD banana"; echo "LCP ADD band"; echo "LCP ADD bandit"; echo "LCP ADD cat"; echo "LCP dog"; echo "LCP GRAPH") | ./t/util/test_helper | dot -Tpng -o images/lcp_tree.png

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
										
