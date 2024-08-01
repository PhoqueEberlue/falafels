Here we notice that setting the proportion threshold of the asynchronous aggregators to 0.9 reduces the performances of configurations using Async algo (in comparison to 10_100_heterogeneous_big_workload that uses 0.5 as a default value).

This can be explained by the fact that the platform is using 50% of "good" computers and 50% of "slow" computers, thus using 0.5 leads to the perfect configuration where the tasks will flow correctly and almost no machines will be waiting for nothing.

It is confirmed by the fact that increasing the proportion makes the results less incredible, even though these configurations are still better.

