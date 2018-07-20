# index_map
index_map is designed to accelerate std::unordered_map (mainly for huge values enumeration) when the key is integer type.
It looks similar with std::unordered_map, but with some differences:
1) Only accept integer type as the key
2) Some interface may be not implemented yet (especially for c++17 and c++20)
3) Make value continuously stored in memory to make value iteration much more efficient 

Here is some performance comparison (latency in micro-second, lower is better, tested on Xeon 6140): 

|                             | unordered_map | index_map | Perf boost |
| --------------------------- | ------------- | --------- | ---------- |
| insert (100000000 elements) |	50279         |	25030	    | 2.0        |
| Iteration (10 times)	      | 84118	        | 542.53	  | 155.0      |
| find&erase (5000 times)	    | 1.628         |	0.542	    | 3.0        |
| operater [] (5000 times)	  | 1.682	        | 0.711     | 2.4        |
