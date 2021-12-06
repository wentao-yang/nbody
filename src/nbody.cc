#include "nbody.h"

#include <getopt.h>

#include <fstream>
#include <iostream>

using namespace std;

bool get_flags(const int& argc, char* const argv[], std::string& test_file_name, 
    bool& random_test, int& num_random_bodies) {
    int command;

    // Get flags
    while ((command = getopt(argc, argv, "f:n:r")) != -1) {
        switch (command) {
            case 'f':
                test_file_name = optarg;
                break;
            case 'n':
                num_random_bodies = atoi(optarg);
                break;
            case 'r':
                random_test = true;
                break;
            case '?':
                if (optopt = 'f') {
                    cerr << "Option -f requires a string.\n";
                } else if (optopt = 'n') {
                    cerr << "Option -n requires an integer.\n";
                } else if (isprint(optopt)) {
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

std::vector<Body> make_bodies(const std::string& test_file_name, const bool& random_test, 
    const int& num_random_bodies) {
    std::vector<Body> bodies;

    if (random_test) {

    } else {
        if (test_file_name.empty()) {
            cerr << "Test file name cannot be empty.\n";
            return -1;
        }

        // Open test file
        ifstream test(test_file_name);
        if (!test.is_open()) {
            cerr << "Could not open the test file: " << test_file_name << ".\n";
            return -1;
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

int main (int argc, char* argv[]) {
    string test_file_name = "";
    bool random_test = false;
    int num_random_bodies = 0;

    if (!get_flags(argc, argv, test_file_name, random_test, num_random_bodies)) {
        return -1;
    }

    vector<Body> bodies = make_bodies(test_file_name, random_test, num_random_bodies);
}
