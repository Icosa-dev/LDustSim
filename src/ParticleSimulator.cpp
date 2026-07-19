/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "ParticleSimulator.h"
#include <raylib.h>

ParticleSimulator::ParticleSimulator() {
    InitWindow(_screenHeight, _screenHeight, "LDustSim");
    SetTargetFPS(120);
}

ParticleSimulator::~ParticleSimulator() {
    if (IsWindowReady())
        CloseWindow();
}
