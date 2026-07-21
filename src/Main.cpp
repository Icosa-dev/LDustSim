/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#define PARTICLE_COUNT 16384

#include "ParticleSimulator.h"

int main(int argc, char **argv) {
    ParticleSimulator simulator = ParticleSimulator(PARTICLE_COUNT);
    simulator.run();
    return 0;
}
