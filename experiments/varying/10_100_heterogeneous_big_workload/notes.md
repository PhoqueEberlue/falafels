Here we can see that the total consumption has a big decrease when adding the first 10 machines.
In the same experiments that uses google's first FL paper workload we don't notice such a decrease because 10 machines is already enough to perform the task.
Whereas here we see that 10 machines is slightly undersized, between 20 and 40 machines we reach a sweet spot, and then it is oversized.

Another thing to note is that RingAsync and StarAsync are performing so well that 10 machines is already well-sized.
The consumption of such configurations are also 3 times less than the others.
Their simulation time are also lower.

However we notice that the energy in Watts (Total consumption / Simulation time) in higher than the rest of the configurations.
It can be explained by the fact that Asynchronous algorithms lead to better task ordering and low synchronization, thus the machines are almost never idle.
This explains that the Power (in Watts) is higher than the others configurations that might have more idle machines in time due to synchronization.
