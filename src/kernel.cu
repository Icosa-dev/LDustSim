/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <raylib.h>

#include "particle.h"
#include "particle_simulator.h"

__global__ void simulation_kernel(const Particle *current_particles,
                                  Particle *next_particles, int num_particles,
                                  float gravity, float delta_time) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= num_particles)
        return;

    Particle p = current_particles[index];

    if (p.is_moveable == 0) {
        next_particles[index] = p;
        return;
    }

    for (int i = 0; i < num_particles; i++) {
        if (i == index)
            continue;

        Particle other = current_particles[i];

        if (other.is_grav_node == 1) {
            float dx = other.x - p.x;
            float dy = other.y - p.y;

            float distance_sq = (dx * dx) + (dy * dy) + 0.1f;
            float distance = sqrtf(distance_sq);

            float force = (other.mass * 10.0f) / distance_sq;

            p.vx += (dx / distance) * force * delta_time;
            p.vy += (dy / distance) * force * delta_time;
        }
    }

    p.x += p.vx * delta_time;
    p.y += p.vy * delta_time;

    next_particles[index] = p;
}

void launch_simulation_kernel(int threads_per_block,
                              const Particle *current_particles,
                              Particle *next_particles, int num_particles,
                              float gravity, float delta_time) {
    int blocks_per_grid = (num_particles + threads_per_block - 1) / threads_per_block;

    simulation_kernel<<<blocks_per_grid, threads_per_block>>>(
        current_particles, next_particles, num_particles, gravity, delta_time);
}

__global__ void render_particles_kernel(const Particle *particles,
                                      int num_particles, Color *pixels,
                                      int width, int height, float max_speed) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= num_particles)
        return;

    Particle p = particles[index];
    int x = (int)p.x;
    int y = (int)p.y;

    if (x >= 0 && x < width && y >= 0 && y < height) {
        float speed = sqrtf(p.vx * p.vx + p.vy * p.vy);
        float normalized_speed = fminf(fmaxf(speed / max_speed, 0.0f), 1.0f);

        Color col = {
            .r = static_cast<unsigned char>(normalized_speed * 255),
            .g = 0,
            .b = static_cast<unsigned char>((1.0f - normalized_speed) * 255),
            .a = 255};

        pixels[y * width + x] = col;
    }
}

void launch_render_particles_kernel(int threads_per_block, const Particle *particles,
                                 int num_particles, Color *pixels, int width,
                                 int height, float max_speed) {
    int blocks_per_grid = (num_particles + threads_per_block - 1) / threads_per_block;

    render_particles_kernel<<<blocks_per_grid, threads_per_block>>>(
        particles, num_particles, pixels, width, height, max_speed);
}
