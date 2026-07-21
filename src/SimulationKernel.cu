/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <math.h>

#include "Particle.h"
#include "ParticleSimulator.h"

__global__ void simulationKernel(Particle *currentParticles,
                                 Particle *nextParticles, int numParticles,
                                 float gravity, float deltaTime) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= numParticles)
        return;

    Particle p = currentParticles[index];

    // TODO: Move all immovable particles to the start of the
    // buffer, so this check is one-and-done
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

void launchSimulationKernel(Particle *currentParticles, Particle *nextParticles,
                            int numParticles, float gravity, float deltaTime) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (numParticles + threadsPerBlock - 1) / threadsPerBlock;

    simulationKernel<<<blocksPerGrid, threadsPerBlock>>>(
        currentParticles, nextParticles, numParticles, gravity, deltaTime);
}
