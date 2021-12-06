#include "nbody.h"

#include <getopt.h>

#include <iostream>
#include <string>

using namespace std;

int main (int argc, char* argv[]) {
    int command;
    string test_file_name = ""; // a string specifying the file of the test
    bool random_test = false; // Instead of a test file, generate random test inputs if true
    int num_random_bodies = 0; // The number of random bodies to generate if `random_test`

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
                return -1;
        }
    }

    cout << "f: " << test_file_name << "\n";
    cout << "n: " << num_random_bodies << "\n";
    cout << "r: " << random_test << "\n";
}
