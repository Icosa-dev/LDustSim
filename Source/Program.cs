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
            ParticleSimulator simulator = new ParticleSimulator(1024);
            simulator.Run();
        }
    }
}
