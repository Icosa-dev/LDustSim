/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

using System.Security;
using ILGPU;

namespace LDustSim
{
    public static class GravitySimulation
    {
        public static void ComputeGravityKernel(
            Index1D index,
            ArrayView<Particle> currentParticles,
            ArrayView<Particle> nextParticles,
            float deltaTime,
            float softeningSquared,
            float G)
        {
            Particle currentParticle = currentParticles[index];

            float ax = 0.0f;
            float ay = 0.0f;

            for (int i = 0; i < currentParticles.Length; i++)
            {
                if (i == index) continue;

                Particle otherParticle = currentParticles[i];

                float dx = otherParticle.X - currentParticle.X;
                float dy = otherParticle.Y - currentParticle.Y;

                float distSquared = dx * dx + dy * dy;

                float invDist = 1.0f / MathF.Sqrt(distSquared);
                float invDistCube = invDist * invDist * invDist;

                float accelerationScalar = otherParticle.Mass * invDistCube;

                ax += dx * accelerationScalar;
                ay += dy * accelerationScalar;
            }

            ax *= G;
            ay *= G;

            currentParticle.VX += ax * deltaTime;
            currentParticle.VY += ay * deltaTime;

            currentParticle.X += currentParticle.VX * deltaTime;
            currentParticle.Y += currentParticle.VY * deltaTime;

            nextParticles[index] = currentParticle;
        }
    }
}
