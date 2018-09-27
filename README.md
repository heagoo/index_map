# index_map
index_map is designed to accelerate std::unordered_map when the key is integer type. 
Here are 2 different implementations:
1) index_map_for_find.h: optimized for find interface, and the map is rarely updated.
2) index_map_for_iteration.h: optimized for value iteration.
It looks similar with std::unordered_map, but with some differences:
1) Only accept integer type as the key
2) Some interface may be not implemented yet (especially for c++17 and c++20)
3) Make values continuously stored in memory to make find and value iteration much more efficient 

Here is the output of bench_find on Xeon 6140: 
```
unordered_map::insert (10000000 elements): 4670.59 ms
unordered_map::find   (10000000 elements): 778.347 ms
unordered_map::insert (10000000 elements): 5266.29 ms
unordered_map::find   (20000000 elements): 1637.92 ms
unordered_map::insert (10000000 elements): 3228.81 ms
unordered_map::find   (30000000 elements): 2774 ms
unordered_map::insert (10000000 elements): 7547.67 ms
unordered_map::find   (40000000 elements): 3268.96 ms
unordered_map::insert (10000000 elements): 3060.38 ms
unordered_map::find   (50000000 elements): 4303.84 ms
unordered_map::insert (10000000 elements): 3360.02 ms
unordered_map::find   (60000000 elements): 5528.85 ms
unordered_map::insert (10000000 elements): 3702.29 ms
unordered_map::find   (70000000 elements): 6867.62 ms
unordered_map::insert (10000000 elements): 12054.3 ms
unordered_map::find   (80000000 elements): 6513.56 ms
unordered_map::insert (10000000 elements): 3040.1 ms
unordered_map::find   (90000000 elements): 7566.01 ms
unordered_map::insert (10000000 elements): 3278.36 ms
unordered_map::find   (100000000 elements): 8864.97 ms
-----------------------------------------------------
    index_map::insert (10000000 elements): 9681.09 ms
    index_map::find   (10000000 elements): 2.718 ms
    index_map::insert (10000000 elements): 10375.2 ms
    index_map::find   (20000000 elements): 5.429 ms
    index_map::insert (10000000 elements): 2906.23 ms
    index_map::find   (30000000 elements): 8.15 ms
    index_map::insert (10000000 elements): 18526.6 ms
    index_map::find   (40000000 elements): 10.859 ms
    index_map::insert (10000000 elements): 1431.68 ms
    index_map::find   (50000000 elements): 13.574 ms
    index_map::insert (10000000 elements): 4703.6 ms
    index_map::find   (60000000 elements): 16.289 ms
    index_map::insert (10000000 elements): 37335.4 ms
    index_map::find   (70000000 elements): 19.003 ms
    index_map::insert (10000000 elements): 1590.68 ms
    index_map::find   (80000000 elements): 21.72 ms
    index_map::insert (10000000 elements): 1549.66 ms
    index_map::find   (90000000 elements): 24.431 ms
    index_map::insert (10000000 elements): 1531.58 ms
    index_map::find   (100000000 elements): 27.144 ms
```

Here is some performance of index_map_for_iteration (latency in micro-second, lower is better, tested on Xeon 6140): 

|                             | unordered_map | index_map | Perf boost |
| --------------------------- | ------------- | --------- | ---------- |
| insert (100000000 elements) |	50279         |	25030	    | 2.0        |
| Iteration (10 times)	      | 84118	        | 542.53	  | 155.0      |
| find&erase (5000 times)	    | 1.628         |	0.542	    | 3.0        |
| operater [] (5000 times)	  | 1.682	        | 0.711     | 2.4        |
