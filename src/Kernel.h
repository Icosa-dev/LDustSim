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
 * @param currentParticles Particle buffer to base calculations on
 * @param nextParticles Output buffer for post-calculation particles
 * @param numParticles Number of particles
 * @param gravity Amount of gravity to simulate
 * @param deltaTime Delta time
 */
void launchSimulationKernel(const Particle *currentParticles,
                            Particle *nextParticles, int numParticles,
                            float gravity, float deltaTime);

/**
 * @brief  Launch a kernel for rendering of particles on the GPU
 *
 * @param particles Particles to render
 * @param numParticles Number of particles
 * @param pixels Pixel buffer to draw on
 * @param width Width of the pixel buffer
 * @param height Height of the pixel buffer
 * @param maxSpeed Max speed of the particles for calculating color based
 * on velocity
 */
void launchRenderParticlesKernel(const Particle *particles, int numParticles,
                                 Color *pixels, int width, int height,
                                 float maxSpeed);
