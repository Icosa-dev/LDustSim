/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "ParticleSimulator.h"

#include <algorithm>
#include <cuda_runtime_api.h>
#include <raylib.h>
#include <vector>

#include "Particle.h"
#include "SimulationKernel.h"

ParticleSimulator::ParticleSimulator(int particleCount)
    : _particleCount(particleCount) {
    cudaMallocManaged((void **)&_bufferA, _particleCount * sizeof(Particle));
    cudaMallocManaged((void **)&_bufferB, _particleCount * sizeof(Particle));

    InitWindow(_screenHeight, _screenHeight, "LDustSim");
    SetTargetFPS(120);
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
