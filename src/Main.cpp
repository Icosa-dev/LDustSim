/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "ParticleSimulator.h"

int main(int argc, char **argv) {
    const int particleCount = 16384;

    ParticleSimulator simulator = ParticleSimulator(particleCount);
    simulator.run();
    return 0;
}
