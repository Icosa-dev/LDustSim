/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include "ParticleSimulator.h"

#include <cmath>
#include <cstdlib>
#include <cuda_runtime_api.h>
#include <random>
#include <raylib.h>
#include <sstream>

#include "Kernel.h"
#include "Particle.h"

ParticleSimulator::ParticleSimulator(int particleCount)
    : _particleCount(particleCount) {
    // Raylib initialization
    InitWindow(_screenWidth, _screenHeight, "LDustSim");
    SetTargetFPS(120);

    // Allocate GPU memory for particle buffers
    cudaMallocManaged((void **)&_bufferA, _particleCount * sizeof(Particle));
    cudaMallocManaged((void **)&_bufferB, _particleCount * sizeof(Particle));

    // Allocate GPU memory for pixel buffer
    cudaMallocManaged((void **)&_cudaPixels,
                      _screenWidth * _screenHeight * sizeof(Color));
    Image screenImage = {.data = _cudaPixels,
                         .width = _screenWidth,
                         .height = _screenHeight,
                         .mipmaps = 1,
                         .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    _screenTexture = LoadTextureFromImage(screenImage);

    // Create central GravNode
    float centerX = static_cast<float>(_screenWidth) / 2.0f;
    float centerY = static_cast<float>(_screenHeight) / 2.0f;

    if (_particleCount > 0) {
        _bufferA[0] = Particle{.x = centerX,
                               .y = centerY,
                               .vx = 0.0f,
                               .vy = 0.0f,
                               .mass = 500000.0f,
                               .isGravNode = 1,
                               .isMoveable = 0};
    }

    // Generate ring of particles around central GravNode
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> distRadius(60.0f, 280.0f);

    for (int i = 1; i < _particleCount; ++i) {
        float angle = distAngle(gen);
        float radius = distRadius(gen);

        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);

        float orbitalSpeed = std::sqrt(5000000.0f / radius);

        float vx = orbitalSpeed * std::sin(angle);
        float vy = -orbitalSpeed * std::cos(angle);

        _bufferA[i] = Particle{.x = x,
                               .y = y,
                               .vx = vx,
                               .vy = vy,
                               .mass = 1.0f,
                               .isGravNode = 0,
                               .isMoveable = 1};
    }

    // Duplicate initial state into buffer B
    for (int i = 0; i < _particleCount; ++i) {
        _bufferB[i] = _bufferA[i];
    }
}

ParticleSimulator::~ParticleSimulator() {
    if (_bufferA) {
        cudaFree(_bufferA);
    }
    if (_bufferB) {
        cudaFree(_bufferB);
    }

    UnloadTexture(_screenTexture);
    if (_cudaPixels) {
        cudaFree(_cudaPixels);
    }

    if (IsWindowReady()) {
        CloseWindow();
    }
}

void ParticleSimulator::run() {
    bool isBufferAInput = true;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        _timeElapsed += deltaTime;

        Particle *inputBuffer = isBufferAInput ? _bufferA : _bufferB;
        Particle *outputBuffer = isBufferAInput ? _bufferB : _bufferA;

        launchSimulationKernel(inputBuffer, outputBuffer, _particleCount,
                               _gravity, deltaTime);

        cudaMemset(_cudaPixels, 0,
                   _screenWidth * _screenHeight * sizeof(Color));

        launchRenderParticlesKernel(outputBuffer, _particleCount, _cudaPixels,
                                    _screenWidth, _screenHeight, _maxSpeed);

        cudaDeviceSynchronize();

        UpdateTexture(_screenTexture, _cudaPixels);

        BeginDrawing();
        {
            ClearBackground(BLACK);
            
            DrawTexture(_screenTexture, 0, 0, WHITE);

            DrawFPS(10, 10);

            std::ostringstream ss;
            ss << "Time Elapsed: " << _timeElapsed;
            DrawText(ss.str().c_str(), 10, 30, 10, GREEN);
        }
        EndDrawing();

        isBufferAInput = !isBufferAInput;
    }
}
