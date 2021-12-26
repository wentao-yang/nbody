#include "nbody.h"

#include <getopt.h>

#include <iostream>

using namespace std;

/**
 * @brief Retrieve command flags.
 * 
 * @param argc argc
 * @param argv argv
 * @param test_type generate random bodies: 0, get bodies from input file: 1
 * @param num_random_bodies number of random bodies to generate if `test_type` = 0
 * @param test_file_name string specifying the file of the test if `test_type` = 1
 * @param implementation sequential: 0, CPU parallel: 1, CUDA: 2
 * @param num_threads number of threads to create if `implementation` = 1, 0 to match 
 * number of bodies
 * @param seconds number of seconds to run the simulation
 * @param output no output: 0, performance: 1, results: 2, all: 3
 * @return true if success; false otherwise
 */
bool get_flags(const int& argc, char* const argv[], int& test_type, int& num_random_bodies, 
    string& test_file_name, int& implementation, int& num_threads, int& seconds, 
    int& output) {
    int command;

    // Get flags one by one
    while ((command = getopt(argc, argv, "t:r:f:i:n:s:o:")) != -1) {
        switch (command) {
            case 't':
                test_type = atoi(optarg);
                break;
            case 'r':
                num_random_bodies = atoi(optarg);
                break;
            case 'f':
                test_file_name = optarg;
                break;
            case 'i':
                implementation = atoi(optarg);
                break;
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 's':
                seconds = atoi(optarg);
                break;
            case 'o':
                output = atoi(optarg);
                break;
            default: // Error
                return false;
        }
    }

    return true;
}

int main (int argc, char* argv[]) {
    // Flags
    int test_type = -1; // generate random bodies: 0, get bodies from input file: 1 -t
    int num_random_bodies = -1; // number of random bodies to generate if `test_type` = 0 -r
    string test_file_name = ""; // string specifying the file of the test if `test_type` = 1 -f
    int implementation = -1; // sequential: 0, CPU parallel: 1, CUDA: 2
    int num_threads = -1; // number of threads to create if `implementation` = 1, 
        // 0 to match number of bodies -n
    int seconds = -1; // number of seconds to run the simulation -s
    int output = 1; // no output: 0, performance: 1, results: 2, all: 3 -o

    if (!get_flags(argc, argv, test_type, num_random_bodies, test_file_name, implementation, 
        num_threads, seconds, output)) {
        cerr << "[ERROR] Getting flags failed.\n";
        return -1;
    }

    NBodySimulator simulator{test_type, num_random_bodies, test_file_name};
    if (!simulator.simulate(implementation, num_threads, seconds, output)) {
        cerr << "[ERROR] Simulation precondition check failed.\n";
        return -1;
    }

    return 0;
}
