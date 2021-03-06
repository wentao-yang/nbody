#include "nbody.h"

#include <math.h>

#include <iostream>

using namespace std;

/**
 * @brief Print the current result of the simulation for the CUDA implementation.
 * 
 * @param positions array of double of the bodies' 3D positions
 * @param radii array of double of bodies' radii
 */
void output_result_cuda(const int num_bodies, 
    const double* const positions, const double* const radii) {
    for (int i = 0; i < num_bodies; i++) {
        cout << positions[i * 3] << " " << positions[i * 3 + 1] << " " << positions[i * 3 + 2] 
            << " " << radii[i] << "\n";
    }
}

/**
 * @brief Update the acceleration array and reset collided boolean for the
 * CUDA implementation.
 * 
 * @param acceleration array of double of the bodies' 3D acceleration
 * @param positions array of double of the bodies' 3D positions
 * @param masses array of double of bodies' masses
 * @param collided array of boolean of whether a body collided this second
 * @param num_bodies the number of bodies in simulation
 */
__global__ void update_acceleration_and_reset_collided_cuda(double* const acceleration, 
    const double* const positions, const double* const masses, bool* const collided, 
    const int num_bodies) {
    int index = threadIdx.x + blockIdx.x * blockDim.x;

    if (index < num_bodies) {
        acceleration[index * 3] = 0;
        acceleration[index * 3 + 1] = 0;
        acceleration[index * 3 + 2] = 0;
        collided[index] = false;

        // Calculate gravitational pull from every other body
        for (int i = 0; i < num_bodies; i++) {
            if (i != index) {
                const double d = sqrt(pow(positions[index * 3] - positions[i * 3], 2) 
                    + pow(positions[index * 3 + 1] - positions[i * 3 + 1], 2)
                    + pow(positions[index * 3 + 2] - positions[i * 3 + 2], 2));
                const double g = (masses[i] * NBodySimulator::G_) / pow(d, 3);

                acceleration[index * 3] += g * (positions[i * 3] - positions[index * 3]);
                acceleration[index * 3 + 1] += g * (positions[i * 3 + 1] - 
                    positions[index * 3 + 1]);
                acceleration[index * 3 + 2] += g * (positions[i * 3 + 2] - 
                    positions[index * 3 + 2]);
            }
        }
    }
}

/**
 * @brief Calculates the new velocity and location of the bodies for the CUDA
 * implementation.
 * 
 * @param positions array of double of the bodies' 3D positions
 * @param velocities array of double of the bodies' 3D velocities
 * @param acceleration array of double of the bodies' 3D acceleration
 * @param collided array of boolean of whether a body collided this second
 * @param num_bodies the number of bodies in simulation
 */
__global__ void update_velocity_and_location_cuda(double* const positions, 
    double* const velocities, const double* const acceleration, const bool* const collided, 
    const int num_bodies) {
    int index = threadIdx.x + blockIdx.x * blockDim.x;

    if (index < num_bodies) {
        if (!collided[index]) {
            velocities[index * 3] += acceleration[index * 3];
            velocities[index * 3 + 1] += acceleration[index * 3 + 1];
            velocities[index * 3 + 2] += acceleration[index * 3 + 2];
        }
        positions[index * 3] += velocities[index * 3];
        positions[index * 3 + 1] += velocities[index * 3 + 1];
        positions[index * 3 + 2] += velocities[index * 3 + 2];
    }
}

/**
 * @brief Handles elastic collisions for the CUDA implementation.
 * 
 * @param velocities array of double of the bodies' 3D velocities
 * @param positions array of double of the bodies' 3D positions
 * @param masses array of double of bodies' masses
 * @param radii array of double of bodies' radii
 * @param collided array of boolean of whether a body collided this second
 * @param num_bodies the number of bodies in simulation
 */
void handle_collisions_cuda(double* const velocities, const double* const positions, 
    const double* const masses, const double* const radii, bool* const collided, const int num_bodies) {
    for (int i = 0; i < num_bodies; i++) {
        for (int j = 0; j < i; j++) {
            if (sqrt(pow(positions[i * 3] - positions[j * 3], 2) + pow(positions[i * 3 + 1] - 
                positions[j * 3 + 1], 2) + pow(positions[i * 3 + 2] - positions[j * 3 + 2], 2)) 
                < (radii[i] + radii[j])) {
                collided[i] = true;
                collided[j] = true;

                // Formula for elastic collision
                const double k1 = (2 * masses[i]) / (masses[i] + masses[j]);
                const double k2 = (masses[i] -  masses[j]) / (masses[i] + masses[j]);
                const double k3 = (2 * masses[j]) / (masses[i] + masses[j]);

                double tmpvel_ = velocities[i * 3];
                velocities[i * 3] = (k2 * tmpvel_) + (k3 * velocities[j * 3]);
                velocities[j * 3] = (k1 * tmpvel_) - (k2 * velocities[j * 3]);

                tmpvel_ = velocities[i * 3 + 1];
                velocities[i * 3 + 1] = (k2 * tmpvel_) + (k3 * velocities[j * 3 + 1]);
                velocities[j * 3 + 1] = (k1 * tmpvel_) - (k2 * velocities[j * 3 + 1]);

                tmpvel_ = velocities[i * 3 + 2];
                velocities[i * 3 + 2] = (k2 * tmpvel_) + (k3 * velocities[j * 3 + 2]);
                velocities[j * 3 + 2] = (k1 * tmpvel_) - (k2 * velocities[j * 3 + 2]);
            }
        }
    }
}

void NBodySimulator::cuda(const vector<Body>& bodies, const int& seconds, const int& output) {
    // Prepare data for GPU
    double *positions, *velocities, *acceleration, *masses, *radii;
    bool *collided;
    cudaMallocManaged(&positions, bodies.size() * sizeof(struct Vector3D));
    cudaMallocManaged(&velocities, bodies.size() * sizeof(struct Vector3D));
    cudaMallocManaged(&acceleration, bodies.size() * sizeof(struct Vector3D));
    cudaMallocManaged(&masses, bodies.size() * sizeof(double));
    cudaMallocManaged(&radii, bodies.size() * sizeof(double));
    cudaMallocManaged(&collided, bodies.size() * sizeof(bool));

    for (int i = 0; i < bodies.size(); i++) {
        positions[i * 3] = bodies[i].pos_.x_;
        positions[i * 3 + 1] = bodies[i].pos_.y_;
        positions[i * 3 + 2] = bodies[i].pos_.z_;
        velocities[i * 3] = bodies[i].vel_.x_;
        velocities[i * 3 + 1] = bodies[i].vel_.y_;
        velocities[i * 3 + 2] = bodies[i].vel_.z_;
        acceleration[i * 3] = bodies[i].acc_.x_;
        acceleration[i * 3 + 1] = bodies[i].acc_.y_;
        acceleration[i * 3 + 2] = bodies[i].acc_.z_;

        masses[i] = bodies[i].mass_;
        radii[i] = bodies[i].radius_;
        collided[i] = bodies[i].collided_;
    }
    
    for (int s = 0; s < seconds; s++) {
        if (output == 2 || output == 3) {
            output_result_cuda(bodies.size(), positions, radii);
        }

        update_acceleration_and_reset_collided_cuda<<<(bodies.size() + THREADS_PER_BLOCK_ - 1) 
            / THREADS_PER_BLOCK_, THREADS_PER_BLOCK_>>>(acceleration, positions, masses, collided, 
            bodies.size());
        cudaDeviceSynchronize();

        handle_collisions_cuda(velocities, positions, masses, radii, collided, bodies.size());

        update_velocity_and_location_cuda<<<(bodies.size() + THREADS_PER_BLOCK_ - 1) 
            / THREADS_PER_BLOCK_, THREADS_PER_BLOCK_>>>(positions, velocities, acceleration, collided, 
            bodies.size());
        cudaDeviceSynchronize();
    }

    if (output == 2 || output == 3) {
        output_result_cuda(bodies.size(), positions, radii);
    }

    cudaFree(positions);
    cudaFree(velocities);
    cudaFree(acceleration);
    cudaFree(masses);
    cudaFree(radii);
    cudaFree(collided);
}
