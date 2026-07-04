/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

namespace LDustSim
{
    /// <summary>
    /// Class <c>Point</c> models a 2-dimentional gravitational particle
    /// </summary>
    public struct Particle
    {
        public float X, Y; // Position
        public float VX, VY; // Velocity
        public float Mass;
        // NOTE: Boolean values are represented as bytes for data computed on
        // the CPU because bools are not blittable
        public byte IsGravNode; // GravNodes interact with other particles
        public byte IsMoveable;
    }
}
