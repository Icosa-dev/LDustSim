/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

using System.Numerics;
using ILGPU;
using ILGPU.Runtime;
using ILGPU.Runtime.CPU;
using ILGPU.Runtime.Cuda;
using Raylib_cs;

namespace LDustSim
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Raylib.InitWindow(1000, 1000, "LDustSim");
            Raylib.SetTargetFPS(60);

            Context context = Context.Create(builder => builder.Default().Cuda());

            using var accelerator = context.GetCudaDevice(0).CreateAccelerator(context);
            Console.WriteLine("Running on: " + accelerator.Name);

            var kernel = accelerator.LoadAutoGroupedStreamKernel<
                Index1D, ArrayView<Particle>, ArrayView<Particle>, float, float, float
            >(GravitySimulation.ComputeGravityKernel);

            int numParticles = 1024;
            Particle[] hostParticles = new Particle[numParticles];

            Random rand = new Random();

            for (int i = 0; i < numParticles; i++)
            {
                hostParticles[i] = new Particle
                {
                    X = (float)rand.NextDouble() * 10,
                    Y = (float)rand.NextDouble() * 10,
                    VX = 0.0f,
                    VY = 0.0f,
                    Mass = 10
                };
            }

            using MemoryBuffer1D<Particle, Stride1D.Dense> bufferA = accelerator.Allocate1D(hostParticles);
            using MemoryBuffer1D<Particle, Stride1D.Dense> bufferOut = accelerator.Allocate1D<Particle>(numParticles);

            MemoryBuffer1D<Particle, Stride1D.Dense> currentInput = bufferA;
            MemoryBuffer1D<Particle, Stride1D.Dense> currentOutput = bufferOut;

            float deltaTime = 0.1f;
            float G = 1.0f;
            float softeningSquared = 0.15f;

            while (!Raylib.WindowShouldClose())
            {
                kernel((Index1D)currentInput.Length, currentInput.View, currentOutput.View, deltaTime, softeningSquared, G);

                var temp = currentInput;
                currentInput = currentOutput;
                currentOutput = temp;

                accelerator.Synchronize();
                currentInput.CopyToCPU(hostParticles);

                Raylib.BeginDrawing();
                Raylib.ClearBackground(Color.Black);

                for (int i = 0; i < numParticles; i++)
                {
                    Raylib.DrawCircle((int)hostParticles[i].X, (int)hostParticles[i].Y, 2.0f, Color.White);
                }

                Raylib.DrawText($"Particles: {numParticles}", 10, 30, 20, Color.Green);

                Raylib.EndDrawing();
            }

            Raylib.CloseWindow();
        }
    }
}
