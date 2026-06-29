/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

using ILGPU;
using ILGPU.Runtime;
using ILGPU.Runtime.Cuda;
using Raylib_cs;

namespace LDustSim
{
    public class ParticleSimulator : IDisposable
    {
        private readonly Context _context;
        private readonly Accelerator _accelerator;
        private MemoryBuffer1D<Particle, Stride1D.Dense> _bufferA;
        private MemoryBuffer1D<Particle, Stride1D.Dense> _bufferB;

        private readonly Action<Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, int, int> _kernel;

        private readonly int _screenWidth = 800;
        private readonly int _screenHeight = 600;
        private readonly int _particleCount;
        private bool _disposed = false;

        private static void SimulationKernel(
            Index1D index,
            ArrayView<Particle> currentParticles,
            ArrayView<Particle> nextParticles,
            float gravity,
            float deltaTime,
            int screenWidth,
            int screenHeight)
        {
            Particle p = currentParticles[index];

            if (p.IsMoveable == 0)
            {
                nextParticles[index] = p;
                return;
            }

            // p.VY += gravity * deltaTime;

            for (int i = 0; i < currentParticles.Length; i++)
            {
                if (i == index) continue;

                Particle other = currentParticles[i];

                if (other.IsGravNode == 1)
                {
                    float dx = other.X - p.X;
                    float dy = other.Y - p.Y;
                    float distanceSq = (dx * dx) + (dy * dy) + 0.1f;
                    float force = (other.Mass * 10.0f) / distanceSq;

                    p.VX += (dx / distanceSq) * force * deltaTime;
                    p.VY += (dy / distanceSq) * force * deltaTime;
                }
            }

            p.X += p.VX * deltaTime;
            p.Y += p.VY * deltaTime;

            nextParticles[index] = p;
        }

        public ParticleSimulator(int particleCount)
        {
            _particleCount = particleCount;

            Raylib.InitWindow(_screenWidth, _screenHeight, "LDustSim");
            Raylib.SetTargetFPS(60);

            _context = Context.Create(builder => builder.Cuda());
            _accelerator = _context.CreateCudaAccelerator(0);

            _bufferA = _accelerator.Allocate1D<Particle>(_particleCount);
            _bufferB = _accelerator.Allocate1D<Particle>(_particleCount);

            _kernel = _accelerator.LoadAutoGroupedStreamKernel<
                Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, int, int
            >(SimulationKernel);

            Particle[] initialParticles = new Particle[_particleCount];
            Random rand = new Random();

            for (int i = 0; i < _particleCount; i++)
            {
                initialParticles[i] = new Particle
                {
                    X = rand.Next(0, _screenWidth),
                    Y = rand.Next(0, _screenHeight),
                    VX = (float)(rand.NextDouble() * 40 - 20),
                    VY = (float)(rand.NextDouble() * 40 - 20),
                    Mass = 1.0f,
                    IsMoveable = 1,
                    IsGravNode = 0
                };
            }

            if (_particleCount > 0)
            {
                initialParticles[0] = new Particle
                {
                    X = _screenWidth / 2f,
                    Y = _screenHeight / 2f,
                    VX = 0,
                    VY = 0,
                    Mass = 500000.0f,
                    IsMoveable = 0,
                    IsGravNode = 1
                };
            }

            _bufferA.CopyFromCPU(initialParticles);

            _bufferB.CopyFromCPU(initialParticles);
        }

        public void Run()
        {
            bool isBufferAInput = true;

            while (!Raylib.WindowShouldClose())
            {
                float deltaTime = Raylib.GetFrameTime();
                float gravity = 9.81f;

                var inputBuffer = isBufferAInput ? _bufferA : _bufferB;
                var outputBuffer = isBufferAInput ? _bufferB : _bufferA;

                _kernel(
                    (int)_bufferA.Length,
                    inputBuffer.View,
                    outputBuffer.View,
                    gravity,
                    deltaTime,
                    _screenWidth,
                    _screenHeight
                );

                _accelerator.Synchronize();

                Particle[] managedParticles = outputBuffer.GetAsArray1D();

                Raylib.BeginDrawing();
                Raylib.ClearBackground(Color.Black);

                foreach (var p in managedParticles)
                {
                    if (p.X >= 0 && p.X < _screenWidth && p.Y >= 0 && p.Y < _screenHeight)
                    {
                        Raylib.DrawPixel((int)p.X, (int)p.Y, Color.White);
                    }
                }

                Raylib.DrawFPS(10, 10);
                Raylib.EndDrawing();

                isBufferAInput = !isBufferAInput;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    _bufferA?.Dispose();
                    _bufferB?.Dispose();
                    _accelerator?.Dispose();
                    _context?.Dispose();

                    if (Raylib.IsWindowReady())
                    {
                        Raylib.CloseWindow();
                    }
                }
                _disposed = true;
            }
        }
    }
}
