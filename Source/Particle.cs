/*
 * Copyright (c) 2026 LJC
 * 
 * SPDX-License-Identifier: MIT
 */

namespace LDustSim
{
    public struct Particle
    {
        public float X, Y;
        public float VX, VY;
        public float Mass;
        public byte IsGravNode;
        public byte IsMoveable;
    }
}
