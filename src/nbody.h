#ifndef NBODY_H
#define NBODY_H

#include <string>
#include <vector>

/**
 * Struct for keeping track of a vector.
 */
struct Vector3D {
    double _x;
    double _y;
    double _z;
};

/**
 * Struct for keeping track of a body.
 */
struct Body {
    Vector3D _pos; // Position
    Vector3D _vel; // Velocity
    double _mass;
    double _radius;
};

/**
 * Retrieve the command flags.
 * 
 * @param argc argc
 * @param argv argv
 * @param test_file_name string specifying the file of the test 
 * @param random_test generate random bodies for test instead of using test file if true
 * @param num_random_bodies number of random bodies to generate if `random_test`
 * @param sequential use sequential implementation if true
 * @param cuda use CUDA implementation if true and `sequential` is false
 * @param num_threads number of threads to create if using non-CUDA parallel implementation
 * @param output output nbody results if true
 * 
 * @return true if success; false otherwise
 */
bool get_flags(const int& argc, char* const argv[], std::string& test_file_name, 
    bool& random_test, int& num_random_bodies, bool& sequential, bool& cuda, int& num_threads,
    bool& output);

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

#endif
