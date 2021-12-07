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
    return sqrt(pow(a.pos_.x_ - b.pos_.x_, 2) + pow(a.pos_.y_ - b.pos_.y_, 2) 
        + pow(a.pos_.z_ - b.pos_.z_, 2));
}

bool collided(const Body& a, const Body& b) {
    return bodies_distance(a, b) < (a.radius_ + b.radius_);
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
            bodies.push_back({{(double) (rand() % 100000) - 50000, (double) 
            (rand() % 100000) - 50000, (double) (rand() % 100000) - 50000}, {0, 0, 0}, 
            {0, 0, 0}, (double) (rand() % 1000) * 1E9, (double) (rand() % 100), false});
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

        test.close();
    }

    return bodies;
}

void update_acceleration_and_reset_collided(vector<Body>& bodies, int body_num) {
    bodies[body_num].acc_.x_ = 0;
    bodies[body_num].acc_.y_ = 0;
    bodies[body_num].acc_.z_ = 0;
    bodies[body_num].collided_ = false;

    for (int i = 0; i < bodies.size(); i++) {
        if (i != body_num) {
            const double d = bodies_distance(bodies[body_num], bodies[i]);
            const double g = (bodies[i].mass_ * G) / pow(d, 3);

            bodies[body_num].acc_.x_ += g * (bodies[i].pos_.x_ 
                - bodies[body_num].pos_.x_);
            bodies[body_num].acc_.y_ += g * (bodies[i].pos_.y_ 
                - bodies[body_num].pos_.y_);
            bodies[body_num].acc_.z_ += g * (bodies[i].pos_.z_ 
                - bodies[body_num].pos_.z_);
        }
    }
}

void handle_collisions(vector<Body>& bodies) {
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = 0; j < i; j++) {
            if (collided(bodies[i], bodies[j])) {
                bodies[i].collided_ = true;
                bodies[j].collided_ = true;

                const double k1 = (2 * bodies[i].mass_) / (bodies[i].mass_ 
                    + bodies[j].mass_);
                const double k2 = (bodies[i].mass_ - bodies[j].mass_) / (bodies[i].mass_ 
                    + bodies[j].mass_);
                const double k3 = (2 * bodies[j].mass_) / (bodies[i].mass_ 
                    + bodies[j].mass_);

                double tmpvel_ = bodies[i].vel_.x_;
                bodies[i].vel_.x_ = (k2 * tmpvel_) + (k3 * bodies[j].vel_.x_);
                bodies[j].vel_.x_ = (k1 * tmpvel_) - (k2 * bodies[j].vel_.x_);

                tmpvel_ = bodies[i].vel_.y_;
                bodies[i].vel_.y_ = (k2 * tmpvel_) + (k3 * bodies[j].vel_.y_);
                bodies[j].vel_.y_ = (k1 * tmpvel_) - (k2 * bodies[j].vel_.y_);

                tmpvel_ = bodies[i].vel_.z_;
                bodies[i].vel_.z_ = (k2 * tmpvel_) + (k3 * bodies[j].vel_.z_);
                bodies[j].vel_.z_ = (k1 * tmpvel_) - (k2 * bodies[j].vel_.z_);
            }
        }
    }
}

void update_velocity_and_location(Body& body) {
    if (!body.collided_) {
        body.vel_ += body.acc_;
    }
    body.pos_ += body.vel_;
}

void output_result(const std::vector<Body>& bodies, const int& second) {
    cout << "Second: " << second << "\n";
    for (int i = 0; i < bodies.size(); i++) {
        cout << "Body: " << i << "\n";
        cout << "Position: (" << bodies[i].pos_.x_ << ", " << bodies[i].pos_.y_ << ", " 
            << bodies[i].pos_.z_ << ")\n";
        cout << "Velocity: (" << bodies[i].vel_.x_ << ", " << bodies[i].vel_.y_ << ", " 
            << bodies[i].vel_.z_ << ")\n";
        cout << "Acceleration: (" << bodies[i].acc_.x_ << ", " << bodies[i].acc_.y_ << 
            ", " << bodies[i].acc_.z_ << ")\n";
    }
    cout << "\n";
}

void nbody_sequential(vector<Body>& bodies, const int& seconds, const bool& output) {
    for (int s = 0; s < seconds; s++) {
        if (output) {
            output_result(bodies, s);
        }

        for (int i = 0; i < bodies.size(); i++) {
            update_acceleration_and_reset_collided(bodies, i);
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

void* nbody_parallel_thread(void* options) {
    const int& thread_number = ((ParallelThreadOption*) options)->thread_number_;
    const int& num_threads = ((ParallelThreadOption*) options)->num_threads_;
    pthread_barrier_t& barrier_ = ((ParallelThreadOption*) options)->barrier_;
    vector<Body>& bodies = ((ParallelThreadOption*) options)->bodies_;
    const int& seconds = ((ParallelThreadOption*) options)->seconds;
    const bool& output = ((ParallelThreadOption*) options)->output;

    const int bodies_size_round_up = ((bodies.size() + num_threads - 1) / num_threads) 
        * num_threads;

    for (int s = 0; s < seconds; s++) {
        if (output && thread_number == 0) {
            output_result(bodies, s);
        }
        pthread_barrier_wait(&barrier_);

        for (int i = thread_number; i < bodies_size_round_up; i+=num_threads) {
            if (i < bodies.size()) {
                update_acceleration_and_reset_collided(bodies, i);
            }
            pthread_barrier_wait(&barrier_);
        }

        if (thread_number == 0) {
            handle_collisions(bodies);
        }
        pthread_barrier_wait(&barrier_);

        for (int i = thread_number; i < bodies_size_round_up; i+=num_threads) {
            if (i < bodies.size()) {
                update_velocity_and_location(bodies[i]);
            }
            pthread_barrier_wait(&barrier_);
        }
    }

    if (output && thread_number == 0) {
        output_result(bodies, seconds);
    }
}

void nbody_parallel(vector<Body>& bodies, const int& seconds, const int& num_threads, 
    const bool& output) {
    const int thread_count = num_threads == 0 ? bodies.size() : num_threads;

    pthread_t threads[thread_count];
    void *status;
    vector<ParallelThreadOption> thread_options;
    thread_options.reserve(thread_count);
    
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, thread_count);

    // Create the threads
    for (int i = 0; i < thread_count; i++) {
        thread_options.push_back({i, thread_count, barrier, bodies, seconds, output});
        if (pthread_create(&threads[i], NULL, nbody_parallel_thread, (void*) 
            &thread_options[i])) {
            cerr << "Unable to create thread " << i << ".\n";
        }
    }

    // Join the threads
    for (int i = 0; i < thread_count; i++) {
        if (pthread_join(threads[i], &status)) {
            cerr << "Unable to join thread " << i << ".\n";
        }
    }
}

int main (int argc, char* argv[]) {
    // Flags
    string test_file_name = ""; // string specifying the file of the test -f
    bool random_test = false; // generate random bodies for test instead of using test file 
        // if true -r
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
        nbody_cuda(bodies, seconds, output);
    } else {
        nbody_parallel(bodies, seconds, num_threads, output);
    }
    chrono::steady_clock::time_point end_time = chrono::steady_clock::now();

    // ms = ns / 1e6
    const double elapsed_ms = chrono::duration_cast<chrono::nanoseconds> (end_time - 
        start_time).count() / 1000000.0;
    printf("Runtime: %lf ms\n", elapsed_ms);

    return 0;
}
