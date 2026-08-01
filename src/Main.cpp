/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <vector>
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "Particle.h"
#include "ParticleSimulator.h"

#define GET_SIM_FIELD(luastate, name, target, getter)                          \
    do {                                                                       \
        lua_getfield(L, -1, name);                                             \
        target = getter(L, -1);                                                \
        lua_pop(L, 1);                                                         \
    } while (0)

#define GET_PARTICLE_FIELD(luastate, key, target, getter)                      \
    GET_SIM_FIELD(luastate, key, target, getter)

int main(int argc, char **argv) {
    // Simulation config values
    double gravity, maxSpeed;
    int targetFPS, screenWidth, screenHeight, particleCount;
    bool showDebugInfo;
    std::vector<Particle> particles;

    // Lua init
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Setup "simulation" table
    lua_newtable(L);

    // Setup inner "particles" table
    lua_newtable(L);
    lua_setfield(L, -2, "particles");

    // Set "simulation" table global
    lua_setglobal(L, "simulation");

    // Load lua script
    if (luaL_dofile(L, argv[1]) != LUA_OK) {
        std::cerr << "Error loading Lua script: " << lua_tostring(L, -1)
                  << std::endl;
        lua_pop(L, 1);
        lua_close(L);
        return 1;
    }

    // Get values from Lua script
    lua_getglobal(L, "simulation");

    // Scalar configurations
    GET_SIM_FIELD(L, "gravity", gravity, lua_tonumber);
    GET_SIM_FIELD(L, "maxSpeed", maxSpeed, lua_tonumber);
    GET_SIM_FIELD(L, "targetFPS", targetFPS, lua_tointeger);
    GET_SIM_FIELD(L, "screenWidth", screenWidth, lua_tointeger);
    GET_SIM_FIELD(L, "screenHeight", screenHeight, lua_tointeger);
    GET_SIM_FIELD(L, "showDebugInfo", showDebugInfo, lua_toboolean);
    GET_SIM_FIELD(L, "particleCount", particleCount, lua_tointeger);

    // Vector configurations
    particles.resize(particleCount);

    lua_getfield(L, -1, "particles");

    for (int i = 1; i <= particleCount; i++) {
        lua_rawgeti(L, -1, i);

        Particle p = particles[i - 1];

        if (lua_istable(L, -1)) {
            GET_PARTICLE_FIELD(L, "x", p.x, lua_tonumber);
            GET_PARTICLE_FIELD(L, "y", p.y, lua_tonumber);
            GET_PARTICLE_FIELD(L, "vx", p.vx, lua_tonumber);
            GET_PARTICLE_FIELD(L, "vy", p.vy, lua_tonumber);
            GET_PARTICLE_FIELD(L, "mass", p.mass, lua_tonumber);
            GET_PARTICLE_FIELD(L, "isGravNode", p.isGravNode, lua_tointeger);
            GET_PARTICLE_FIELD(L, "isMoveable", p.isMoveable, lua_tointeger);
        }

        lua_pop(L, 1);
    }

    lua_pop(L, 2);

    lua_close(L);

    ParticleSimulator simulator =
        ParticleSimulator(particles.data(), particleCount, gravity, maxSpeed,
                          screenWidth, screenHeight, targetFPS, showDebugInfo);
    simulator.run();

    return 0;
}
