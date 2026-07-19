/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstddef>

struct Particle {
    float x, y;
    float vx, vy;
    float mass;
    std::byte isGravNode;
    std::byte isMoveable;
};
