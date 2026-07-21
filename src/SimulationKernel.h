/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

struct Particle;

void launchSimulationKernel(Particle *currentParticles, Particle *nextParticles,
                            int numParticles, float gravity, float deltaTime);
