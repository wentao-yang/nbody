#include "nbody.h"

#include <getopt.h>
#include <math.h>
#include <pthread.h> 
#include <stdlib.h>

#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;

double bodies_distance(const Body& a, const Body& b) {
    return sqrt(pow(a._pos._x - b._pos._x, 2) + pow(a._pos._y - b._pos._y, 2) 
        + pow(a._pos._z - b._pos._z, 2));
}

bool collided(const Body& a, const Body& b) {
    return bodies_distance(a, b) < (a._radius + b._radius);
}

bool get_flags(const int& argc, char* const argv[], string& test_file_name, 
    bool& random_test, int& num_random_bodies, bool& sequential, bool& cuda, 
    int& num_threads, int& seconds, bool& output) {
    int command;

    // Get flags
    while ((command = getopt(argc, argv, "f:n:t:x:rsco")) != -1) {
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
            case 'x':
                seconds = atoi(optarg);
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
                } else if (optopt = 'x') {
                    cerr << "Option -x requires an integer.\n";
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

vector<Body> make_bodies(const string& test_file_name, const bool& random_test, 
    const int& num_random_bodies) {
    vector<Body> bodies;

    if (random_test) {
        for (int i = 0; i < num_random_bodies; i++) {
            double x, y, z, mass, radius;
            bodies.push_back({{(double) (rand() % 100000) - 50000, (double) (rand() % 100000) - 50000, 
            (double) (rand() % 100000) - 50000}, {0, 0, 0}, {0, 0, 0}, (double) (rand() % 1000) * 1E9, 
            (double) (rand() % 100), false});
        }
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
            bodies.push_back({{x, y, z}, {0, 0, 0}, {0, 0, 0}, mass, radius, false});
        }
    }

    return bodies;
}

void update_acceleration(vector<Body>& bodies, int body_num) {
    bodies[body_num]._acc._x = 0;
    bodies[body_num]._acc._y = 0;
    bodies[body_num]._acc._z = 0;
    bodies[body_num]._collided = false;

    for (int i = 0; i < bodies.size(); i++) {
        if (i != body_num) {
            double d = bodies_distance(bodies[body_num], bodies[i]);
            double g = (bodies[i]._mass * G) / (pow(d, 3));

            bodies[body_num]._acc._x += g * (bodies[i]._pos._x - bodies[body_num]._pos._x);
            bodies[body_num]._acc._y += g * (bodies[i]._pos._y - bodies[body_num]._pos._y);
            bodies[body_num]._acc._z += g * (bodies[i]._pos._z - bodies[body_num]._pos._z);
        }
    }
}

void handle_collisions(vector<Body>& bodies) {
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = 0; j < i; j++) {
            if (collided(bodies[i], bodies[j])) {
                bodies[i]._collided = true;
                bodies[j]._collided = true;

                double k1 = (2 * bodies[i]._mass) / (bodies[i]._mass + bodies[j]._mass);
                double k2 = (bodies[i]._mass - bodies[j]._mass) / (bodies[i]._mass + bodies[j]._mass);
                double k3 = (2 * bodies[j]._mass) / (bodies[i]._mass + bodies[j]._mass);

                double tmp_vel = bodies[i]._vel._x;
                bodies[i]._vel._x = (k2 * tmp_vel) + (k3 * bodies[j]._vel._x);
                bodies[j]._vel._x = (k1 * tmp_vel) - (k2 * bodies[j]._vel._x);

                tmp_vel = bodies[i]._vel._y;
                bodies[i]._vel._y = (k2 * tmp_vel) + (k3 * bodies[j]._vel._y);
                bodies[j]._vel._y = (k1 * tmp_vel) - (k2 * bodies[j]._vel._y);

                tmp_vel = bodies[i]._vel._z;
                bodies[i]._vel._z = (k2 * tmp_vel) + (k3 * bodies[j]._vel._z);
                bodies[j]._vel._z = (k1 * tmp_vel) - (k2 * bodies[j]._vel._z);
            }
        }
    }
}

void update_velocity_and_location(Body& body) {
    if (!body._collided) {
        body._vel += body._acc;
    }
    body._pos += body._vel;
}

void output_result(const std::vector<Body>& bodies, const int& second) {
    cout << "Second: " << second << "\n";
    for (int i = 0; i < bodies.size(); i++) {
        cout << "Body: " << i << "\n";
        cout << "Position: (" << bodies[i]._pos._x << ", " << bodies[i]._pos._y << ", " 
            << bodies[i]._pos._z << ")\n";
        cout << "Velocity: (" << bodies[i]._vel._x << ", " << bodies[i]._vel._y << ", " 
            << bodies[i]._vel._z << ")\n";
        cout << "Acceleration: (" << bodies[i]._acc._x << ", " << bodies[i]._acc._y << ", " 
            << bodies[i]._acc._z << ")\n";
    }
    cout << "\n";
}

void nbody_sequential(vector<Body>& bodies, const int& seconds, const bool& output) {
    for (int s = 0; s < seconds; s++) {
        if (output) {
            output_result(bodies, s);
        }

        for (int i = 0; i < bodies.size(); i++) {
            update_acceleration(bodies, i);
        }

        handle_collisions(bodies);

        for (Body& b : bodies) {
            update_velocity_and_location(b);
        }
    }

    if (output) {
        output_result(bodies, seconds);
    }
}

void nbody_parallel(std::vector<Body>& bodies, const int& seconds, const int& num_threads, 
    const bool& output) {
    for (int s = 0; s < seconds; s++) {
        if (output) {
            output_result(bodies, s);
        }

        for (int i = 0; i < bodies.size(); i++) {
            update_acceleration(bodies, i);
        }

        handle_collisions(bodies);

        for (Body& b : bodies) {
            update_velocity_and_location(b);
        }
    }

    if (output) {
        output_result(bodies, seconds);
    }
}

int main (int argc, char* argv[]) {
    // Flags
    string test_file_name = ""; // string specifying the file of the test -f
    bool random_test = false; // generate random bodies for test instead of using test file if true -r
    int num_random_bodies = 0; // number of random bodies to generate if `random_test` -n
    bool sequential = false; // use sequential implementation if true -s
    bool cuda = false; // use CUDA implementation if true and `sequential` is false -c
    int num_threads = 0; // number of threads to create if using CPU parallel implementation -t
    int seconds = 0; // number of seconds to run the simulation -x
    bool output = false; // output nbody results if true -o

    if (!get_flags(argc, argv, test_file_name, random_test, num_random_bodies, sequential, 
        cuda, num_threads, seconds, output)) {
        cerr << "Getting flags failed.\n";
        return -1;
    }

    vector<Body> bodies = make_bodies(test_file_name, random_test, num_random_bodies);
    if (bodies.size() == 0) {
        cerr << "Creating bodies failed.\n";
        return -1;
    }

    chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
    if (sequential) {
        nbody_sequential(bodies, seconds, output);
    } else if (cuda) {
        // TODO(wentaoyang): CUDA implementation
        cerr << "CUDA implementation not yet implemented.\n";
        return -1;
    } else {
        // TODO(wentaoyang): CPU parallel implementation
        cerr << "CPU parallel implementation not yet implemented.\n";
        return -1;
    }
    chrono::steady_clock::time_point end_time = chrono::steady_clock::now();

    // ms = ns / 1e6
    double elapsed_ms = chrono::duration_cast<chrono::nanoseconds> (end_time - start_time).count() / 1000000.0;
    printf("Runtime: %lf ms\n", elapsed_ms);

    return 0;
}
