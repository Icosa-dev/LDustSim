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
        private float _elapsedTime = 0.0f;
        private bool _disposed = false;

        private Particle _p1;

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

            for (int i = 0; i < currentParticles.Length; i++)
            {
                if (i == index) continue;

                Particle other = currentParticles[i];

                if (other.IsGravNode == 1)
                {
                    float dx = other.X - p.X;
                    float dy = other.Y - p.Y;

                    float distanceSq = (dx * dx) + (dy * dy) + 0.1f;
                    float distance = MathF.Sqrt(distanceSq);

                    float force = (other.Mass * 10.0f) / distanceSq;

                    p.VX += (dx / distance) * force * deltaTime;
                    p.VY += (dy / distance) * force * deltaTime;
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
            Raylib.SetTargetFPS(120);

            _context = Context.Create(builder => builder.Cuda());
            _accelerator = _context.CreateCudaAccelerator(0);

            _bufferA = _accelerator.Allocate1D<Particle>(_particleCount);
            _bufferB = _accelerator.Allocate1D<Particle>(_particleCount);

            _kernel = _accelerator.LoadAutoGroupedStreamKernel<
                Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, int, int
            >(SimulationKernel);

            Particle[] initialParticles = new Particle[_particleCount];
            Random rand = new Random();

            float centerX = _screenWidth / 2f;
            float centerY = _screenHeight / 2f;

            if (_particleCount > 0)
            {
                initialParticles[0] = new Particle
                {
                    X = centerX,
                    Y = centerY,
                    VX = 0,
                    VY = 0,
                    Mass = 500000.0f,
                    IsMoveable = 0,
                    IsGravNode = 1
                };
            }

            for (int i = 1; i < _particleCount; i++)
            {
                float angle = (float)(rand.NextDouble() * 2.0 * Math.PI);

                float radius = (float)(rand.NextDouble() * 220 + 60);

                float x = centerX + radius * (float)Math.Cos(angle);
                float y = centerY + radius * (float)Math.Sin(angle);

                float orbitalSpeed = (float)Math.Sqrt(5000000.0f / radius);

                float vx = orbitalSpeed * (float)Math.Sin(angle);
                float vy = -orbitalSpeed * (float)Math.Cos(angle);

                initialParticles[i] = new Particle
                {
                    X = x,
                    Y = y,
                    VX = vx,
                    VY = vy,
                    Mass = 1.0f,
                    IsMoveable = 1,
                    IsGravNode = 0
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

                _elapsedTime += deltaTime;

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

                if (managedParticles.Length > 1)
                {
                    _p1 = managedParticles[1];
                }

                Raylib.BeginDrawing();
                Raylib.ClearBackground(Color.Black);

                foreach (var p in managedParticles)
                {
                    float radius = Math.Max(1.0f, (float)Math.Sqrt(p.Mass) * 0.05f);

                    if (p.X >= -radius && p.X < _screenWidth && p.Y >= -radius && p.Y < _screenHeight)
                    {
                        float speed = (float)Math.Sqrt((p.VX * p.VX) + (p.VY * p.VY));

                        float maxSpeed = 300f;
                        float normalizedSpeed = Math.Clamp(speed / maxSpeed, 0f, 1f);

                        byte r = (byte)(normalizedSpeed * 255);
                        byte g = 0;
                        byte b = (byte)((1.0f - normalizedSpeed) * 255);
                        Color particleColor = new Color(r, g, b, (byte)255);

                        if (p.IsGravNode == 1)
                        {
                            particleColor = Color.Yellow;
                        }

                        Raylib.DrawCircle((int)p.X, (int)p.Y, radius, particleColor);
                    }
                }

                Raylib.DrawFPS(10, 10);
                Raylib.DrawText("Particle Count: " + _particleCount, 10, 30, 10, Color.Green);
                Raylib.DrawText("P1 Position: (" + _p1.X + ", " + _p1.Y + ")", 10, 40, 10, Color.Green);
                Raylib.DrawText("P1 Velocity: (" + _p1.VX + ", " + _p1.VY + ")", 10, 50, 10, Color.Green);
                Raylib.DrawText("Time Elapsed: " + _elapsedTime + "s", 10, 60, 10, Color.Green);
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
