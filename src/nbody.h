#ifndef NBODY_H
#define NBODY_H

#include <string>
#include <vector>

/**
 * @brief Struct for keeping track of a 3D vector.
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
 * @brief Struct for keeping track of a body.
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
 * @brief Class to for nbody simulation.
 */
class NBodySimulator {
  public:
    /**
     * @brief Construct a new NBodySimulator object and prepare the bodies.
     * 
     * @param test_type generate random bodies: 0, get bodies from input file: 1
     * @param num_random_bodies number of random bodies to generate if `test_type` = 0
     * @param test_file_name string specifying the file of the test if `test_type` = 1
     */
    NBodySimulator(int test_type, int num_random_bodies, std::string test_file_name);

    /**
     * @brief Attempt the nbody simulation and printing the appropriate information.
     * This method is safe to be repeated.
     * 
     * @param implementation sequential: 0, CPU parallel: 1, CUDA: 2
     * @param num_threads number of threads to create if `implementation` = 1, 0 to match 
     * number of bodies
     * @param seconds number of seconds to run the simulation
     * @param output no output: 0, performance: 1, results: 2, all: 3
     * @return true if preconditions pass; false otherwise
     */
    bool simulate(int implementation, int num_threads, int seconds, int output);

    static constexpr double G_ = 6.67408E-11; // Gravitational constant

  private:
    /**
     * @brief Get the Euclidean distance between two bodies.
     * 
     * @param a a body
     * @param b another body
     * @return the Euclidean distance between the two bodies 
     */
    static double bodies_distance(const Body& a, const Body& b);

    /**
     * @brief Checks if two bodies are collided.
     * 
     * @param a a body
     * @param b another body
     * @return true if the bodies are collided.
     */
    static bool collided(const Body& a, const Body& b);

    /**
     * @brief Print the current result of the simulation at this simulation second.
     * 
     * @param bodies vector of the bodies
     * @param second current second of simulation
     */
    static void output_result(const std::vector<Body>& bodies, const int& second);

    /**
     * @brief Calculates the new acceleration of `bodies[body_num]` and reset collided 
     * boolean for all bodies.
     * 
     * @param bodies vector of the bodies
     * @param body_num the index of the body in `bodies`
     */
    static void update_acceleration_and_reset_collided(std::vector<Body>& bodies, 
        const int& body_num);

    /**
     * @brief Handles elastic collisions.
     * 
     * @param bodies vector of the bodies
     */
    static void handle_collisions(std::vector<Body>& bodies);

    /**
     * @brief Calculates the new velocity and location of `body`.
     * 
     * @param body body to be updated
     */
    static void update_velocity_and_location(Body& body);

    /**
     * @brief Sequential implementation.
     * 
     * @param seconds number of seconds to run the simulation
     * @param output no output: 0, performance: 1, results: 2, all: 3
     */
    void sequential(std::vector<Body> bodies, const int& seconds, const int& output);

    /**
     * @brief Struct for the options of a pthread for the CPU parallel implementation.
     */
    struct CPUParallelThreadOption {
        const int thread_number_;
        const int& num_threads_;
        pthread_barrier_t& barrier_;
        std::vector<Body>& bodies_;
        const int& seconds;
        const int& output;
    };

    /**
     * @brief Function for a single thread in the CPU parallel implementation.
     * 
     * @param options pointer to the ParallelThreadOption of this thread
     */ 
    static void* cpu_parallel_thread(void* options);

    /**
     * @brief CPU parallel implementation.
     * 
     * @param bodies vector of the bodies
     * @param num_threads number of threads to create if using CPU parallel implementation
     * @param seconds number of seconds to run the simulation
     * @param output write nbody results to std::cout if true
     */
    void cpu_parallel(std::vector<Body> bodies, const int& num_threads, const int& seconds, 
        const int& output);

    /**
     * @brief Basic CUDA implementation.
     * 
     * @param bodies vector of the bodies
     * @param seconds number of seconds to run the simulation
     * @param output write nbody results to std::cout if true
     */
    void cuda(const std::vector<Body>& bodies, const int& seconds, const bool& output);

    static constexpr int THREADS_PER_BLOCK_ = 512; // For CUDA

    std::vector<Body> bodies_;
};

#endif
