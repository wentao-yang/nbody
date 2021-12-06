#ifndef NBODY_H
#define NBODY_H

#include <string>
#include <vector>

// Gravitational constant
const double G = 6.67408E-11;

/**
 * Struct for keeping track of a vector.
 */
struct Vector3D {
    double x_;
    double y_;
    double z_;

    Vector3D operator += (Vector3D const &other) {
        this->x_ += other.x_;
        this->y_ += other.y_;
        this->z_ += other.z_;
        return *this;
    }
};

/**
 * Struct for keeping track of a body.
 */
struct Body {
    Vector3D pos_; // Position
    Vector3D vel_; // Velocity
    Vector3D acc_; // Acceleration
    double mass_;
    double radius_;
    bool collided_; // Have this body collided with another
};

/**
 * Struct for the options of a pthread.
 */
struct ParallelThreadOption {
    const int thread_number_;
    const int& num_threads_;
    pthread_barrier_t& barrier_;
    std::vector<Body>& bodies_;
    const int& seconds;
    const bool& output;
};

/**
 * Get the Euclidean distance between two bodies.
 * 
 * @param a a body
 * @param b another body
 */
double bodies_distance(const Body& a, const Body& b);

/**
 * Checks if two bodies are collided.
 * 
 * @param a a body
 * @param b another body
 * 
 * @return true if the bodies are collided.
 */
bool collided(const Body& a, const Body& b);

/**
 * Retrieve command flags.
 * 
 * @param argc argc
 * @param argv argv
 * @param test_file_name string specifying the file of the test 
 * @param random_test generate random bodies for test instead of using test file if true
 * @param num_random_bodies number of random bodies to generate if `random_test`
 * @param sequential use sequential implementation if true
 * @param cuda use CUDA implementation if true and `sequential` is false
 * @param num_threads number of threads to create if using CPU parallel implementation
 * @param seconds number of seconds to run the simulation
 * @param output write nbody results to std::cout if true
 * 
 * @return true if success; false otherwise
 */
bool get_flags(const int& argc, char* const argv[], std::string& test_file_name, 
    bool& random_test, int& num_random_bodies, bool& sequential, bool& cuda, int& num_threads,
    int& seconds, bool& output);

/**
 * Generate the necessary bodies for n-bodies.
 * 
 * @param test_file_name string specifying the file of the test 
 * @param random_test generate random bodies for test instead of using test file if true
 * @param num_random_bodies number of random bodies to generate if `random_test`
 * 
 * @return vector of the generated bodies
 */
std::vector<Body> make_bodies(const std::string& test_file_name, const bool& random_test, 
    const int& num_random_bodies);

/**
 * Calculates the new acceleration of `bodies[body_num]`.
 * 
 * @param bodies vector of the bodies
 * @param body_num the index of the body in `bodies`
 */
void update_acceleration(std::vector<Body>& bodies, int body_num);

/**
 * Handles elastic collisions.
 * 
 * @param bodies vector of the bodies
 */
void handle_collisions(std::vector<Body>& bodies);

/**
 * Calculates the new velocity and location of `body`.
 * 
 * @param body body to be updated
 */
void update_velocity_and_location(Body& body);

/**
 * Print the current result of the simulation.
 * 
 * @param bodies vector of the bodies
 * @param second current second of simulation
 */
void output_result(const std::vector<Body>& bodies, const int& second);

/**
 * Sequential implementation.
 * 
 * @param bodies vector of the bodies
 * @param seconds number of seconds to run the simulation
 * @param output write nbody results to std::cout if true
 */
void nbody_sequential(std::vector<Body>& bodies, const int& seconds, const bool& output);

/**
 * Function for a single thread in the CPU parallel implementation.
 * 
 * @param options pointer to the ParallelThreadOption of this thread
 */ 
void* nbody_parallel_thread(void* options);

/**
 * CPU parallel implementation.
 * 
 * @param bodies vector of the bodies
 * @param seconds number of seconds to run the simulation
 * @param num_threads number of threads to create if using CPU parallel implementation
 * @param output write nbody results to std::cout if true
 */
void nbody_parallel(std::vector<Body>& bodies, const int& seconds, const int& num_threads, 
    const bool& output);

#endif
