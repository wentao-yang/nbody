#include "nbody.h"

#include <getopt.h>

#include <fstream>
#include <iostream>

using namespace std;

bool get_flags(const int& argc, char* const argv[], string& test_file_name, 
    bool& random_test, int& num_random_bodies, bool& sequential, bool& cuda, 
    int& num_threads, bool& output) {
    int command;

    // Get flags
    while ((command = getopt(argc, argv, "f:n:t:rsco")) != -1) {
        switch (command) {
            case 'f':
                test_file_name = optarg;
                break;
            case 'n':
                num_random_bodies = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'r':
                random_test = true;
                break;
            case 's':
                sequential = true;
                break;
            case 'c':
                cuda = true;
                break;
            case 'o':
                output = true;
                break;
            case '?':
                if (optopt = 'f') {
                    cerr << "Option -f requires a string.\n";
                } else if (optopt = 'n') {
                    cerr << "Option -n requires an integer.\n";
                } else if (optopt = 't') {
                    cerr << "Option -t requires an integer.\n";
                }else if (isprint(optopt)) {
                    cerr << "Unknown option: " << optopt << ".\n";
                } else {
                    cerr << "Unknown character: " << optopt << ".\n";
                }
            default:
                return false;
        }
    }
    return true;
}

vector<Body> make_bodies(const string& test_file_name, const bool& random_test, 
    const int& num_random_bodies) {
    vector<Body> bodies;

    if (random_test) {
        cerr << "Random bodies generation not yet implemented.\n";
    } else {
        if (test_file_name.empty()) {
            cerr << "Test file name cannot be empty.\n";
            return bodies;
        }

        // Open test file
        ifstream test(test_file_name);
        if (!test.is_open()) {
            cerr << "Could not open the test file: " << test_file_name << ".\n";
            return bodies;
        }

        int num_bodies;
        test >> num_bodies;

        for (int i = 0; i < num_bodies; i++) {
            double x, y, z, mass, radius;
            test >> x >> y >> z >> mass >> radius;
            bodies.push_back({{x, y, z}, {0, 0, 0}, mass, radius});
        }
    }

    return bodies;
}

bool nbody_sequential(vector<Body>& bodies, const bool& output) {
    return true;
}

int main (int argc, char* argv[]) {
    // Flags
    string test_file_name = ""; // string specifying the file of the test -f
    bool random_test = false; // generate random bodies for test instead of using test file if true -r
    int num_random_bodies = 0; // number of random bodies to generate if `random_test` -n
    bool sequential = false; // use sequential implementation if true -s
    bool cuda = false; // use CUDA implementation if true and `sequential` is false -c
    int num_threads = 0; // number of threads to create if using CPU parallel implementation -t
    bool output = false; // output nbody results if true -o

    if (!get_flags(argc, argv, test_file_name, random_test, num_random_bodies, sequential, 
        cuda, num_threads, output)) {
        cerr << "Getting flags failed.\n";
        return -1;
    }

    vector<Body> bodies = make_bodies(test_file_name, random_test, num_random_bodies);
    if (bodies.size() == 0) {
        cerr << "Creating bodies failed.\n";
        return -1;
    }

    if (sequential) {
        if (!nbody_sequential(bodies, output)) {
            cerr << "Sequential implementation failed.\n";
            return -1;
        }
    } else if (cuda) {
        cerr << "CUDA implementation not yet implemented.\n";
        return -1;
    } else {
        cerr << "CPU parallel implementation not yet implemented.\n";
        return -1;
    }

    return 0;
}
