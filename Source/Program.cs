/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

// TODO: Document all of the code

namespace LDustSim
{
    public class Program
    {
        public static void Main(string[] args)
        {
            ParticleSimulator simulator = new ParticleSimulator(16_384);
            simulator.Run();
        }
    }
}
