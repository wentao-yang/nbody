#include "nbody.h"

#include <math.h>
#include <pthread.h> 

#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;

NBodySimulator::NBodySimulator(int test_type, int num_random_bodies, string test_file_name) {
    if (test_type == 0) { // Generate random bodies
        if (num_random_bodies <= 0) {
            cerr << "[ERROR] `num_random_bodies` should be above 0. Actual: " << 
                num_random_bodies << "\n";
        }

        for (int i = 0; i < num_random_bodies; i++) {
            bodies_.push_back({{(double) (rand() % 100000) - 50000, (double) 
            (rand() % 100000) - 50000, (double) (rand() % 100000) - 50000}, {0, 0, 0}, 
            {0, 0, 0}, (double) (rand() % 1000) * 1E9, (double) (rand() % 100), false});
        }
    } else if (test_type == 1) { // Get bodies from input file
        if (test_file_name.empty()) {
            cerr << "[ERROR] `test_file_name` should not be empty.\n";
            return;
        }

        // Open test file
        ifstream file(test_file_name);
        if (!file.is_open()) {
            cerr << "[ERROR] Could not open the test file: " << test_file_name << ".\n";
            return;
        }

        int num_bodies;
        file >> num_bodies;

        for (int i = 0; i < num_bodies; i++) {
            double x, y, z, mass, radius;
            file >> x >> y >> z >> mass >> radius;
            bodies_.push_back({{x, y, z}, {0, 0, 0}, {0, 0, 0}, mass, radius, false});
        }

        file.close();
    } else {
        cerr << "[ERROR] `test_type` should be either 0 or 1. Actual: " << test_type << "\n";
    }
}

bool NBodySimulator::simulate(int implementation, int num_threads, int seconds, int output) {
    // Check preconditions
    if (bodies_.size() == 0) {
        cerr << "[ERROR] The number of bodies is 0.\n";
        return false;
    }
    if (seconds <= 0) {
        cerr << "[ERROR] `seconds` should be above 0. Actual: " << seconds << "\n";
        return false;
    }
    if (output < 0 || output > 3) {
        cerr << "[ERROR] `output` should be between [0, 3]. Actual: " << output << "\n";
        return false;
    }

    chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
    if (implementation == 0) {
        sequential(bodies_, seconds, output);
    } else if (implementation == 1) {
        if (num_threads < 0) {
            cerr << "[ERROR] `num_threads` should be greater than or equal to 0. Actual: " 
                << num_threads << "\n";
            return false;
        }
        cpu_parallel(bodies_, num_threads, seconds, output);
    } else if (implementation == 2) {
        cuda(bodies_, seconds, output);
    } else {
        cerr << "[ERROR] `implementation` should be between [0, 2]. Actual: " << implementation 
            << "\n";
        return false;
    }
    chrono::steady_clock::time_point end_time = chrono::steady_clock::now();

    if (output == 1 || output == 3) {
        // ms = ns / 1e6
        const double elapsed_ms = chrono::duration_cast<chrono::nanoseconds> (end_time - 
            start_time).count() / 1000000.0;
        printf("Runtime: %lf ms\n", elapsed_ms);
    }

    return true;
}

double NBodySimulator::bodies_distance(const Body& a, const Body& b) {
    return sqrt(pow(a.pos_.x_ - b.pos_.x_, 2) + pow(a.pos_.y_ - b.pos_.y_, 2) 
        + pow(a.pos_.z_ - b.pos_.z_, 2));
}

bool NBodySimulator::collided(const Body& a, const Body& b) {
    return bodies_distance(a, b) < (a.radius_ + b.radius_);
}

void NBodySimulator::output_result(const vector<Body>& bodies, const int& second) {
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

void NBodySimulator::update_acceleration_and_reset_collided(vector<Body>& bodies, 
    const int& body_num) {
    bodies[body_num].acc_.x_ = 0;
    bodies[body_num].acc_.y_ = 0;
    bodies[body_num].acc_.z_ = 0;
    bodies[body_num].collided_ = false;

    // Calculate gravitational pull from every other body
    for (int i = 0; i < bodies.size(); i++) {
        if (i != body_num) {
            const double d = bodies_distance(bodies[body_num], bodies[i]);
            const double g = (bodies[i].mass_ * G_) / pow(d, 3);

            bodies[body_num].acc_.x_ += g * (bodies[i].pos_.x_ 
                - bodies[body_num].pos_.x_);
            bodies[body_num].acc_.y_ += g * (bodies[i].pos_.y_ 
                - bodies[body_num].pos_.y_);
            bodies[body_num].acc_.z_ += g * (bodies[i].pos_.z_ 
                - bodies[body_num].pos_.z_);
        }
    }
}

void NBodySimulator::handle_collisions(vector<Body>& bodies) {
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = 0; j < i; j++) {
            if (collided(bodies[i], bodies[j])) {
                bodies[i].collided_ = true;
                bodies[j].collided_ = true;

                // Using formula for elastic collision
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

void NBodySimulator::update_velocity_and_location(Body& body) {
    if (!body.collided_) {
        body.vel_ += body.acc_;
    }
    body.pos_ += body.vel_;
}

void NBodySimulator::sequential(vector<Body> bodies, const int& seconds, const int& output) {
    for (int s = 0; s < seconds; s++) {
        if (output == 2 || output == 3) {
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

    if (output == 2 || output == 3) {
        output_result(bodies, seconds);
    }
}

void* NBodySimulator::cpu_parallel_thread(void* options) {
    const int& thread_number = ((CPUParallelThreadOption*) options)->thread_number_;
    const int& num_threads = ((CPUParallelThreadOption*) options)->num_threads_;
    pthread_barrier_t& barrier_ = ((CPUParallelThreadOption*) options)->barrier_;
    vector<Body>& bodies = ((CPUParallelThreadOption*) options)->bodies_;
    const int& seconds = ((CPUParallelThreadOption*) options)->seconds;
    const bool& output = ((CPUParallelThreadOption*) options)->output;

    // `bodies_size_round_up` is the next multiple of num_threads larger than bodies.size()
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

void NBodySimulator::cpu_parallel(vector<Body> bodies, const int& num_threads, const int& seconds, 
    const int& output) {
    const int thread_count = num_threads == 0 ? bodies.size() : num_threads;

    pthread_t threads[thread_count];
    void *status;
    vector<CPUParallelThreadOption> thread_options;
    thread_options.reserve(thread_count);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, thread_count);

    // Create the threads
    for (int i = 0; i < thread_count; i++) {
        thread_options.push_back({i, thread_count, barrier, bodies, seconds, output});
        if (pthread_create(&threads[i], NULL, cpu_parallel_thread, (void*) 
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
