CPPFLAGS = -O2 -std=c++11 -Wall -Wextra -Werror

all: test bench_find bench_iteration

test: test.cpp index_map_for_find.h
	g++ test.cpp -o test $(CPPFLAGS)

bench_find: bench_find.cpp index_map_for_find.h
	g++ bench_find.cpp -o bench_find $(CPPFLAGS)

bench_iteration: bench_iteration.cpp index_map_for_iteration.h
	g++ bench_iteration.cpp -o bench_iteration -O2 -std=c++11

clean:
	rm -f test bench_find bench_iteration
