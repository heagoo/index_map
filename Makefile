all: test test_perf

test: test.cpp index_map.h 
	g++ test.cpp -o test -O2 -std=c++11

test_perf: test_perf.cpp index_map.h 
	g++ test_perf.cpp -o test_perf -O2 -std=c++11

clean:
	rm -f test test_perf
