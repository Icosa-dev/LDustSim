/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>

struct Particle;

/**
 * @brief Simulates and renders a simulation of particles
 */
class ParticleSimulator {
private:
    // Simulation buffers
    Particle *buffer_a, *buffer_b;

    // Simulation variables
    const int particle_count;
    const float gravity;
    float time_elapsed = 0.0f;
    const float max_speed;
    const int simulation_threads_per_block;

    // Raylib variables
    const int screen_width;
    const int screen_height;
    const int target_fps;
    const bool show_debug_info;

    // GPU rendering fields
    Color *cuda_pixels = nullptr;
    Texture2D screen_texture;
    const int renderer_threads_per_block;

public:
    /**
     * @brief Construct a new Particle Simulator object
     *
     * @param particles Initial particle buffer
     * @param particle_count Total number of particles
     * @param gravity Gravitational constant
     * @param max_speed Max speed a particle can reach
     * @param screen_width Simulation screen width
     * @param screen_height Simulation screen height
     * @param target_fps Target FPS for the simulation
     * @param show_debug_info Simulation will display debug info if true
     * @param simulation_threads_per_block Number of threads the simulation kernel
     * will use
     * @param renderer_threads_per_block Number of threads the renderer kernel will
     * use
     */
    ParticleSimulator(Particle *particles, int particle_count, float gravity,
                      float max_speed, int screen_width, int screen_height,
                      int target_fps, bool show_debug_info,
                      int simulation_threads_per_block,
                      int renderer_threads_per_block);

    /**
     * @brief Destroy the Particle Simulator object
     */
    ~ParticleSimulator();

    /**
     * @brief Run a simulation
     */
    void run();
};
