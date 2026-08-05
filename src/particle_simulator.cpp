/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "particle_simulator.h"

#include <cmath>
#include <cstdlib>
#include <cuda_runtime_api.h>
#include <random>
#include <raylib.h>
#include <sstream>

#include "kernel.h"
#include "particle.h"

ParticleSimulator::ParticleSimulator(Particle *particles, int particle_count,
                                     float gravity, float max_speed,
                                     int screen_width, int screen_height,
                                     int target_fps, bool show_debug_info,
                                     int simulation_threads_per_block,
                                     int renderer_threads_per_block)
    : particle_count(particle_count), gravity(gravity), max_speed(max_speed),
      screen_width(screen_width), screen_height(screen_height),
      target_fps(target_fps), show_debug_info(show_debug_info),
      simulation_threads_per_block(simulation_threads_per_block),
      renderer_threads_per_block(renderer_threads_per_block) {
    // Raylib initialization
    InitWindow(screen_width, screen_height, "LDustSim");
    SetTargetFPS(120);

    // Allocate GPU memory for particle buffers
    cudaMallocManaged((void **)&buffer_a, particle_count * sizeof(Particle));
    cudaMallocManaged((void **)&buffer_b, particle_count * sizeof(Particle));

    // Allocate GPU memory for pixel buffer
    cudaMallocManaged((void **)&cuda_pixels,
                      screen_width * screen_height * sizeof(Color));
    Image screen_image = {.data = cuda_pixels,
                          .width = screen_width,
                          .height = screen_height,
                          .mipmaps = 1,
                          .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    screen_texture = LoadTextureFromImage(screen_image);

    // Duplicate initial state into buffers
    for (int i = 0; i < particle_count; ++i) {
        buffer_a[i] = particles[i];
        buffer_b[i] = particles[i];
    }
}

ParticleSimulator::~ParticleSimulator() {
    if (buffer_a) {
        cudaFree(buffer_a);
    }
    if (buffer_b) {
        cudaFree(buffer_b);
    }

    UnloadTexture(screen_texture);
    if (cuda_pixels) {
        cudaFree(cuda_pixels);
    }

    if (IsWindowReady()) {
        CloseWindow();
    }
}

void ParticleSimulator::run() {
    bool is_buffer_a_input = true;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        time_elapsed += delta_time;

        Particle *inputBuffer = is_buffer_a_input ? buffer_a : buffer_b;
        Particle *outputBuffer = is_buffer_a_input ? buffer_b : buffer_a;

        launch_simulation_kernel(simulation_threads_per_block, inputBuffer,
                                 outputBuffer, particle_count, gravity,
                                 delta_time);

        cudaMemset(cuda_pixels, 0,
                   screen_width * screen_height * sizeof(Color));

        launch_render_particles_kernel(renderer_threads_per_block, outputBuffer,
                                       particle_count, cuda_pixels,
                                       screen_width, screen_height, max_speed);

        cudaDeviceSynchronize();

        UpdateTexture(screen_texture, cuda_pixels);

        BeginDrawing();
        {
            ClearBackground(BLACK);
            DrawTexture(screen_texture, 0, 0, WHITE);

            if (show_debug_info) {
                DrawFPS(10, 10);
            }
        }
        EndDrawing();

        is_buffer_a_input = !is_buffer_a_input;
    }
}
