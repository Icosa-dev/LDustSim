/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

namespace LDustSim
{
    public class Program
    {
        public static void Main(string[] args)
        {
            using ParticleSimulator simulator = new ParticleSimulator(16_384);
            simulator.Run();
        }
    }
}
