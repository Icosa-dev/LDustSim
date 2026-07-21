/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "ParticleSimulator.h"

#include <cmath>
#include <cstdlib>
#include <random>
#include <cuda_runtime_api.h>
#include <raylib.h>

#include "Particle.h"
#include "SimulationKernel.h"

ParticleSimulator::ParticleSimulator(int particleCount)
    : _particleCount(particleCount) {
    InitWindow(_screenHeight, _screenHeight, "LDustSim");
    SetTargetFPS(120);

    cudaMallocManaged((void **)&_bufferA, _particleCount * sizeof(Particle));
    cudaMallocManaged((void **)&_bufferB, _particleCount * sizeof(Particle));

    float centerX = static_cast<float>(_screenWidth) / 2.0f;
    float centerY = static_cast<float>(_screenHeight) / 2.0f;

    // Create central GravNode
    if (_particleCount > 0) {
        _bufferA[0] = Particle{
            .x = centerX,
            .y = centerY,
            .vx = 0.0f,
            .vy = 0.0f,
            .mass = 500000.0f,
            .isGravNode = 1,
            .isMoveable = 0 
        };
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> distRadius(
        60.0f, 280.0f);

    for (int i = 1; i < _particleCount; ++i) {
        float angle = distAngle(gen);
        float radius = distRadius(gen);

        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);

        float orbitalSpeed = std::sqrt(5000000.0f / radius);

        float vx = orbitalSpeed * std::sin(angle);
        float vy = -orbitalSpeed * std::cos(angle);

        _bufferA[i] = Particle{
            .x = x,
            .y = y,
            .vx = vx,
            .vy = vy,
            .mass = 1.0f,
            .isGravNode = 0,
            .isMoveable = 1
        };
    }

    // Duplicate initial state into buffer B
    for (int i = 0; i < _particleCount; ++i) {
        _bufferB[i] = _bufferA[i];
    }
}

ParticleSimulator::~ParticleSimulator() {
    if (_bufferA)
        cudaFree(_bufferA);
    if (_bufferB)
        cudaFree(_bufferB);

    if (IsWindowReady()) {
        CloseWindow();
    }
}

void ParticleSimulator::run() {
    bool isBufferAInput = true;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        _elapsedTime += deltaTime;

        Particle *inputBuffer = isBufferAInput ? _bufferA : _bufferB;
        Particle *outputBuffer = isBufferAInput ? _bufferB : _bufferA;

        launchSimulationKernel(inputBuffer, outputBuffer, _particleCount,
                               _gravity, deltaTime);

        cudaDeviceSynchronize();

        BeginDrawing();
        ClearBackground(BLACK);

        for (size_t i = 0; i < _particleCount; ++i) {
            DrawPixel(outputBuffer[i].x, outputBuffer[i].y, WHITE);
        }

        DrawFPS(10, 10);
        EndDrawing();

        isBufferAInput = !isBufferAInput;
    }
}
