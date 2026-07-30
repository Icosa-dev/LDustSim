/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>

struct Particle;

/**
 * @brief Simulates and renders a simulation of particles
 */
class ParticleSimulator {
private:
    // Simulation buffers
    Particle *_bufferA, *_bufferB;

    // Simulation variables
    const int _particleCount;
    float _gravity = 9.81f;
    float _timeElapsed = 0.0f;
    // Particle *_p1;
    float _maxSpeed = 300.0f;

    // Raylib variables
    const int _screenWidth = 720;
    const int _screenHeight = 900;

    // GPU rendering fields
    Color *_cudaPixels = nullptr;
    Texture2D _screenTexture;

public:
    /**
     * @brief Construct a new Particle Simulator object
     *
     * @param particleCount The number of particles to simulate
     */
    ParticleSimulator(int particleCount);

    /**
     * @brief Destroy the Particle Simulator object
     */
    ~ParticleSimulator();

    /**
     * @brief Run a simulation
     */
    void run();
};
