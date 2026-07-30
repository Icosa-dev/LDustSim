/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>

struct Particle;

void launchSimulationKernel(Particle *currentParticles, Particle *nextParticles,
                            int numParticles, float gravity, float deltaTime);

void launchRenderParticlesKernel(const Particle *particles,
                                      int numParticles, Color *pixels,
                                      int width, int height, float maxSpeed);
