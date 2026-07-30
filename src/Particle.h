/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>

struct Particle {
    float x, y;
    float vx, vy;
    float mass;
    uint8_t isGravNode; // 1 if influences other particles
    uint8_t isMoveable;
};
