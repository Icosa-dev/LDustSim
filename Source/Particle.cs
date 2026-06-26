/*
 * Copyright (c) 2026 LJC
 * 
 * SPDX-License-Identifier: MIT
 */

namespace LDustSim
{
    public struct Particle
    {
        // Simulation Attributes
        public float X, Y;
        public float VX, VY;
        public float Mass;
        public bool isGravNode;
        public bool isMoveable;

        // Rendering Attributes
        public float Size;
    }
}
