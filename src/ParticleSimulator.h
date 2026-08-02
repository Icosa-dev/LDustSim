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
    const float _gravity;
    float _elapsedTime = 0.0f;
    const float _maxSpeed;
    const int _simulationThreadsPerBlock;

    // Raylib variables
    const int _screenWidth;
    const int _screenHeight;
    const int _targetFPS;
    const bool _showDebugInfo;

    // GPU rendering fields
    Color *_cudaPixels = nullptr;
    Texture2D _screenTexture;
    const int _rendererThreadsPerBlock;

public:
    /**
     * @brief Construct a new Particle Simulator object
     *
     * @param particles Initial particle buffer
     * @param particleCount Total number of particles
     * @param gravity Gravitational constant
     * @param maxSpeed Max speed a particle can reach
     * @param screenWidth Simulation screen width
     * @param screenHeight Simulation screen height
     * @param targetFPS Target FPS for the simulation
     * @param showDebugInfo Simulation will display debug info if true
     * @param simulationThreadsPerBlock Number of threads the simulation kernel will use
     * @param rendererThreadsPerBlock Number of threads the renderer kernel will use
     */
    ParticleSimulator(Particle *particles, int particleCount, float gravity,
                      float maxSpeed, int screenWidth, int screenHeight,
                      int targetFPS, bool showDebugInfo, int simulationThreadsPerBlock, int rendererThreadsPerBlock);

    /**
     * @brief Destroy the Particle Simulator object
     */
    ~ParticleSimulator();

    /**
     * @brief Run a simulation
     */
    void run();
};
