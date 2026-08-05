/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>

struct Particle;

/**
 * @brief Launch a kernel for gravitational simulation of particles on
 * the GPU
 *
 * @param threads_per_blcok The number of threads per block that the kernel will
 * use
 * @param current_particles Particle buffer to base calculations on
 * @param next_particles Output buffer for post-calculation particles
 * @param num_particles Number of particles
 * @param gravity Amount of gravity to simulate
 * @param delta_time Delta time
 */
void launch_simulation_kernel(int threads_per_block,
                              const Particle *current_particles,
                              Particle *next_particles, int num_particles,
                              float gravity, float delta_time);

/**
 * @brief  Launch a kernel for rendering of particles on the GPU
 *
 * @param threads_per_block The number of threads per block that the kernel will
 * use
 * @param particles Particles to render
 * @param num_particles Number of particles
 * @param pixels Pixel buffer to draw on
 * @param width Width of the pixel buffer
 * @param height Height of the pixel buffer
 * @param max_speed Max speed of the particles for calculating color based
 * on velocity
 */
void launch_render_particles_kernel(int threads, const Particle *particles,
                                    int num_particles, Color *pixels, int width,
                                    int height, float max_speed);
