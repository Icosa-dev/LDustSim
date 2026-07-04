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
    /// <summary>
    /// Class <c>ParticleSimulator</c> simulates and renders 2D gravitational
    /// particles
    /// </summary>
    public class ParticleSimulator : IDisposable
    {
        // ILGPU variables
        private readonly Context _context;
        private readonly Accelerator _accelerator;

        // Simulation buffers
        private MemoryBuffer1D<Particle, Stride1D.Dense> _bufferA;
        private MemoryBuffer1D<Particle, Stride1D.Dense> _bufferB;

        // Simulation kernel variable
        private readonly Action<Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, int, int> _kernel;

        // Raylib variables
        private readonly int _screenWidth = 720;
        private readonly int _screenHeight = 900;

        // Simulation variables
        private readonly int _particleCount;
        private float _elapsedTime = 0.0f;
        private Particle _p1; // Track the state of particle 1

        private bool _disposed = false;

        /// <summary>
        /// Constructs a <c>ParticleSimulator</c> object
        /// </summary>
        /// <param name="particleCount">The number of particles to simulate</param>
        public ParticleSimulator(int particleCount)
        {
            _particleCount = particleCount;

            // Initialize Raylib
            Raylib.InitWindow(_screenWidth, _screenHeight, "LDustSim");
            Raylib.SetTargetFPS(120);

            // Initialize ILGPU for CUDA
            // TODO: Allow for use of non-NVIDIA GPUs
            _context = Context.Create(builder => builder.Cuda());
            _accelerator = _context.CreateCudaAccelerator(0);

            // Initialize buffers
            _bufferA = _accelerator.Allocate1D<Particle>(_particleCount);
            _bufferB = _accelerator.Allocate1D<Particle>(_particleCount);

            // Load kernel
            _kernel = _accelerator.LoadAutoGroupedStreamKernel<
                Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, int, int
            >(SimulationKernel);

            // Initialize particles in buffers
            Particle[] initialParticles = new Particle[_particleCount];
            Random rand = new Random();

            // Create GravNode in center
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
                    Mass = 500_000f,
                    IsGravNode = 1, // true
                    IsMoveable = 0, // false
                };
            }

            // For the rest of the particles, make a ring around the GravNode
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
                    IsMoveable = 1, // Moved by central GravNode
                    IsGravNode = 0 // Do not move other particles
                };
            }

            // Copy particles into buffers
            _bufferA.CopyFromCPU(initialParticles);
            _bufferB.CopyFromCPU(initialParticles);
        }

        /// <summary>
        /// Run a particle simulation
        /// </summary>
        public void Run()
        {
            bool isBufferAInput = true; // Switch between A and B for double buffering

            while (!Raylib.WindowShouldClose())
            {
                float deltaTime = Raylib.GetFrameTime();
                float gravity = 9.81f; // TODO: this should be an object variable

                _elapsedTime += deltaTime;

                // Set input and output buffer
                var inputBuffer = isBufferAInput ? _bufferA : _bufferB;
                var outputBuffer = isBufferAInput ? _bufferB : _bufferA;

                // Run kernel on GPU
                _kernel(
                    (int)_bufferA.Length,
                    inputBuffer.View,
                    outputBuffer.View,
                    gravity,
                    deltaTime,
                    _screenWidth,
                    _screenHeight
                );

                // Wait for kernel execution
                _accelerator.Synchronize();

                // Copy outputBuffer data from GPU to CPU
                Particle[] managedParticles = outputBuffer.GetAsArray1D();

                if (managedParticles.Length > 1)
                {
                    _p1 = managedParticles[1];
                }

                // Draw particles
                Raylib.BeginDrawing();
                Raylib.ClearBackground(Color.Black);

                foreach (var p in managedParticles)
                {
                    // Render radius proportional to mass
                    float radius = Math.Max(1.0f, (float)Math.Sqrt(p.Mass) * 0.05f);

                    if (p.X >= -radius && p.X < _screenWidth && p.Y >= -radius && p.Y < _screenHeight)
                    {
                        float speed = (float)Math.Sqrt((p.VX * p.VX) + (p.VY * p.VY));

                        float maxSpeed = 300f;
                        float normalizedSpeed = Math.Clamp(speed / maxSpeed, 0f, 1f);

                        // Render color proportional to speed or yellow for GravNodes
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

                // Draw debug info    
                Raylib.DrawFPS(10, 10);
                Raylib.DrawText("Particle Count: " + _particleCount, 10, 30, 10, Color.Green);
                Raylib.DrawText("P1 Position: (" + _p1.X + ", " + _p1.Y + ")", 10, 40, 10, Color.Green);
                Raylib.DrawText("P1 Velocity: (" + _p1.VX + ", " + _p1.VY + ")", 10, 50, 10, Color.Green);
                Raylib.DrawText("Time Elapsed: " + _elapsedTime + "s", 10, 60, 10, Color.Green);
                Raylib.EndDrawing();

                // Swap input buffer
                isBufferAInput = !isBufferAInput;
            }
        }

        /// <summary>
        /// Dispose of memory use for simulation
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Helper method for disposing simulation memory
        /// </summary>
        /// <param name="disposing">True if simulation memory is being disposed, false otherwise</param>
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

        /// <summary>
        /// GPU kernel for calculating gravitational forces on each particle
        /// </summary>
        /// <param name="index">Index of current particle</param>
        /// <param name="currentParticles">Input particle buffer</param>
        /// <param name="nextParticles">Output particle buffer</param>
        /// <param name="gravity">Simulation gravitational constant</param>
        /// <param name="deltaTime">Simulation delta time</param>
        /// <param name="screenWidth">Width of the simulation screen</param>
        /// <param name="screenHeight">Height of the simulation screen</param>
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

            // Skip physics simulation on immovable particles
            if (p.IsMoveable == 0)
            {
                nextParticles[index] = p;
                return;
            }

            // Physics simulation on movable particles
            for (int i = 0; i < currentParticles.Length; i++)
            {
                // Skip interaction with self
                if (i == index) continue;

                Particle other = currentParticles[i];

                // Interact with GravNodes
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

            // Set new positional values
            p.X += p.VX * deltaTime;
            p.Y += p.VY * deltaTime;

            // Update coresponding particle in output buffer
            nextParticles[index] = p;
        }
    }
}
