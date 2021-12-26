# N-body Simulation

## Background and Goals

The *n-body* problem is important in physics in understanding how objects interact with each other via its laws, including the conservation of energy and momentum and Newton's Law of Universal Gravitation. This project uses *C++* and *CUDA* to tackle this problem and uses *Python* and *Matplotlib* to visualize the results.

The code is expected to either take in a file that is formatted appropriately to generate the initial state of the bodies or generate a chosen amount of bodies with random parameters. Then, the code will perform the simulation on the bodies, with a possible 3 ways: sequentially, parallel using CPU, and using *CUDA*. The sequential method, as the name suggests, does not utilize any parallel implementations and serves as a baseline, and the parallel implementation using the CPU allows for a manual number of threads. The final implementation is expected to use *CUDA*. The simulation has a set number of seconds to be simulated and will be timed so that the efficiency of each method can be compared. 

For the simulation, Newton's Law of Universal Gravitation is expected to be maintained. Additionally, when there is a collision, both energy and momentum are expected to be conserved, meaning that there will not be any absorption of bodies.

## Methodology and Tricks

### The Gist

The juice of the code for the simulation is encapsulated in the `NBodySimulator` class in `src/nbody.h`. This class forms the model from a test file or generates random bodies itself, simulates the scenario, and prints any output to the console. Simulations can be rerun with the same `NBodySimulator` object with different parameters, such as the implementation type to use. 

The three implementations follow a similar order for simulating every body every second:

1. Calculate the total acceleration felt by the body from every other body in the system by iterating over every other body and calculating the gravitational acceleration using the formula from Newton's Law of Universal Gravitation between the two.
2. Use the acceleration value to modify the velocity, which is a simple addition as the time step is 1 second.
3. Use the new velocity to modify the position, which is also a simple addition as the time step is 1 second.

There, however, exists a case where two bodies touch each other/collide. In this special case, the code will ignore the acceleration value generated for both bodies and calculate their new velocity as if an elastic collision occurred between the two by derivation from the formulas from the conservation of momentum and energy. Acceleration is ignored because the gravitational force between the two may be too strong due to them being close together, holding the two bodies together, which is not the elastic collisions where the two bodies would "bounce off" each other desired.

### Sequential Method

The sequential implementation performs exactly as described in "The Gist" section and is used as a baseline to compare the other two implementations.

### CPU Parallel Method

For this implementation, *pthread* was used to create threads and join them together. The approach is *SIMD* and similar to the sequential method, except in the final acceleration, velocity, and position calculations, threads are created to parallelize the instructions and the sequential part that remained is handled by thread 0. A barrier is used when the threads need to be on the same page before proceeding. 

### CUDA Method

The *CUDA* implementation is similar to the CPU parallel implementation if the number of bodies count of threads is dispatched, but instead of threads on the CPU, it will be threads in thread blocks on the GPU. Additionally, the data had to be converted into simple arrays, which led to a bit of overhead. Furthermore, instead of a barrier, *cudaDeviceSynchronize* was used. 

### Visualization

The visualization is done using *Python* and *Matplotlib*. The code takes in the result output of the simulation via pipe and converts it into lists of *Numpy* arrays that contain every body's position for every second. Then,  *Matplotlib* is used to animate the results by illustrating plots as frames. The visualization is saved as a *.gif* file if needed.

### File Structure

The source code is in the folder $src$, and two small tests and their expected results are in $tests/$ and $expected\_test\_results/$ respectively. The two tests are for simple demonstrations rather than intensive testing. A Makefile exists to compile and run the code.

## Evaluation and Results

Results were collected by running single trials on a machine with an Intel(R) Xeon(R) Gold 6226R CPU processor. The *CUDA* code was run on an Nvidia Quadro RTX 6000 with Turing architecture. The host machine's OS version is Ubuntu 18.04. For each body-time scenario/test, 5 samples were taken. All implementations' results were tested with each other to ensure accuracy. The runtime results graph below will be analyzed.

![Performance Runtime Test Graph](https://github.com/wentao-yang/nbody/blob/main/figures/runtime_graph.PNG?raw=true)

In this figure, the total runtime for the different scenarios is shown. Although it might be difficult to see in this graph, the general pattern is that while the sequential and CPU paralleled methods are increasing in a quadratic fashion if the number of bodies or simulation time is changed, the *CUDA* implementation remains almost linear. On the other hand, the *CUDA* implementation performs worse at low body count and simulation time, such as 1000 bodies; 30 seconds. This is most likely due to the overhead when converting the data into the necessary linear arrays for *CUDA*. One surprising result is that despite the high number of bodies, N threads remained efficient compared to the other methods all the way through. Normally, a high number of threads might accrue large amounts of overhead, but perhaps the Xeon CPU's high core count allowed N threads to prevail in this testing environment. 2 threads, however, fell shorter even to the 1 threaded paralleled and sequential implementations as perhaps this is where the barrier starts to become active. 

To finalize the results, the N threaded CPU parallel implementation reigned supreme in the tests, while the CUDA implementation came in at second. The former appears to be increasing at a faster pace than the latter, so at a large enough body count/simulation time combination, the CUDA implementation will be faster than any of the other implementations. 2 threads performed the worst, while 1 thread matched the sequential implementation in terms of performance. It gets better after 2, as 4, 8, and 16 threads all performed better than their lower thread count counterparts. 

Here is the visualization for a scenario with 100 random bodies simulated for 600 seconds:

![Visualization for Scenario with 100 Random Bodies for 600 Seconds](https://github.com/wentao-yang/nbody/blob/main/figures/random_r100_s600_results.gif?raw=true)

## Build Instructions

The commands to run the code are in the `Makefile`. Use `make compile` to create an executable named `nbody` callable with the flags below. For the visualization, use the command `python3 src/visualize.py` and pipe the results file into this command.

### Flags for `nbody`

- *-t* int : type of test; generate random bodies: 0, get bodies from input file: 1
- *-r* int : number of random bodies to generate if the *-t* flag is set to 0
- *-f* string : location of the test file if the *-t* flag is set to 1
- *-i* int : implementation; sequential: 0, CPU parallel: 1, CUDA: 2
- *-n* int : number of threads to generate if the *-i* flag is set to 1; match the number of bodies: 0
- *-s* int : number of seconds to run the simulation
- *-o* int : output amount; no output: 0, performance output only: 1, results output only: 2, all outputs: 3

### Flags for `src/visualize.py`

- *-s* : save the visualization as a gif to `figures/nbody.gif` if set
