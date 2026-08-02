# LDustSim

LDustSim is a CUDA GPU-Accelerated N-Body gravitational particle simulation.

## Building

```
git clone https://github.com/Icosa-dev/LDustSim.git
cd LDustSim
mkdir build
cmake -B build
cd build
make
./LDustSim
```

## Scripting

Simulations can be customized via lua scripts. The path to the configuring
lua script can be passed as the first argument to the program. The following
are the variables which can be customized. Note that all must be set to a value
otherwise the program will throw an error. The preset simulation scripts are
located in `LDustSim/src/Scripts.h` as macros.

- `simulation.gravity`: Gravitational constant for the simulation (float)
- `simulation.maxSpeed`: Max speed for any given particle (float)
- `simulation.screenWidth`: Width of the simulation display (int)
- `simulation.screenHeight`: Height of the simulation display (int)
- `simulation.targetFPS`: Target FPS of the simulation (int)
- `simulation.showDebugInfo`: If true, extra display info is displayed in the top left (bool)
- `simulation.threadsPerBlock`: Threads per block for the simulation kernel (int)
- `simulation.rendererThreadsPerBlock`: Threads per block for the renderer kernel (int)
- `simulation.particleCount`: Total number of particles to simulate (int)
- `simulation.particles`: Array of particles to simulate and render (particle[])

Each particle is stored as a table in the following format:
- `x, y`: Positional values (float)
- `vx, vy`: Velocity values (float)
- `mass`: Mass of the particle (float)
- `isGravNode`: If true, the particle will affect other particles
- `isMoveable`: If true, the particle will be affected by other particles
