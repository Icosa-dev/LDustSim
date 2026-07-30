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

#include "Particle.h"
#include "ParticleSimulator.h"

__global__ void simulationKernel(const Particle *currentParticles,
                                 Particle *nextParticles, int numParticles,
                                 float gravity, float deltaTime) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= numParticles)
        return;

    Particle p = currentParticles[index];

    if (p.isMoveable == 0) {
        nextParticles[index] = p;
        return;
    }

    for (int i = 0; i < numParticles; i++) {
        if (i == index)
            continue;

        Particle other = currentParticles[i];

        if (other.isGravNode == 1) {
            float dx = other.x - p.x;
            float dy = other.y - p.y;

            float distanceSq = (dx * dx) + (dy * dy) + 0.1f;
            float distance = sqrtf(distanceSq);

            float force = (other.mass * 10.0f) / distanceSq;

            p.vx += (dx / distance) * force * deltaTime;
            p.vy += (dy / distance) * force * deltaTime;
        }
    }

    p.x += p.vx * deltaTime;
    p.y += p.vy * deltaTime;

    nextParticles[index] = p;
}

void launchSimulationKernel(const Particle *currentParticles, Particle *nextParticles,
                            int numParticles, float gravity, float deltaTime) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (numParticles + threadsPerBlock - 1) / threadsPerBlock;

    simulationKernel<<<blocksPerGrid, threadsPerBlock>>>(
        currentParticles, nextParticles, numParticles, gravity, deltaTime);
}

__global__ void renderParticlesKernel(const Particle *particles,
                                      int numParticles, Color *pixels,
                                      int width, int height, float maxSpeed) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= numParticles)
        return;

    Particle p = particles[index];
    int x = (int)p.x;
    int y = (int)p.y;

    if (x >= 0 && x < width && y >= 0 && y < height) {
        float speed = sqrtf(p.vx * p.vx + p.vy * p.vy);
        float normalizedSpeed = fminf(fmaxf(speed / maxSpeed, 0.0f), 1.0f);

        Color col = {
            .r = static_cast<unsigned char>(normalizedSpeed * 255),
            .g = 0,
            .b = static_cast<unsigned char>((1.0f - normalizedSpeed) * 255),
            .a = 255};

        pixels[y * width + x] = col;
    }
}

void launchRenderParticlesKernel(const Particle *particles, int numParticles,
                                 Color *pixels, int width, int height,
                                 float maxSpeed) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (numParticles + threadsPerBlock - 1) / threadsPerBlock;

    renderParticlesKernel<<<blocksPerGrid, threadsPerBlock>>>(
        particles, numParticles, pixels, width, height, maxSpeed);
}
