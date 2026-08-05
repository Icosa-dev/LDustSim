/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

#define VERISON "INDEV"

#include <argparse/argparse.hpp>
#include <iostream>
#include <vector>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "particle.h"
#include "particle_simulator.h"
#include "scripts.h"

#define GET_SIM_FIELD(luastate, name, target, getter)                          \
    do {                                                                       \
        lua_getfield(L, -1, name);                                             \
        target = getter(L, -1);                                                \
        lua_pop(L, 1);                                                         \
    } while (0)

#define GET_PARTICLE_FIELD(luastate, key, target, getter)                      \
    GET_SIM_FIELD(luastate, key, target, getter)

int main(int argc, char **argv) {
    // Argparsing
    argparse::ArgumentParser program("LDustSim", VERISON);

    program.add_argument("configuration-script")
        .help("Path to a .lua file to define the nature of the simulation")
        .default_value(std::string(""))
        .nargs(0, 1);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    std::string config_file = program.get<std::string>("configuration-script");

    // Simulation config values
    double gravity, max_speed;
    int target_fps, screen_width, screen_height, particle_count,
        simulation_threads_per_block, renderer_threads_per_block;
    bool show_debug_info;
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

    auto handle_lua_error = [L]() {
        std::cerr << "Error loading Lua script: " << lua_tostring(L, -1)
                  << std::endl;
        lua_pop(L, 1);
        lua_close(L);
        return 1;
    };

    if (config_file == "") {
        if (luaL_dostring(L, DEFAULT_SIMULATION) != LUA_OK) {
            return handle_lua_error();
        }
    } else if (luaL_dofile(L, config_file.c_str()) != LUA_OK) {
        return handle_lua_error();
    }

    // Get values from Lua script
    lua_getglobal(L, "simulation");

    // Scalar configurations
    GET_SIM_FIELD(L, "gravity", gravity, lua_tonumber);
    GET_SIM_FIELD(L, "maxSpeed", max_speed, lua_tonumber);
    GET_SIM_FIELD(L, "targetFPS", target_fps, lua_tointeger);
    GET_SIM_FIELD(L, "screenWidth", screen_width, lua_tointeger);
    GET_SIM_FIELD(L, "screenHeight", screen_height, lua_tointeger);
    GET_SIM_FIELD(L, "showDebugInfo", show_debug_info, lua_toboolean);
    GET_SIM_FIELD(L, "particleCount", particle_count, lua_tointeger);
    GET_SIM_FIELD(L, "threadsPerBlock", simulation_threads_per_block,
                  lua_tointeger);
    GET_SIM_FIELD(L, "rendererThreadsPerBlock", renderer_threads_per_block,
                  lua_tointeger);

    // Vector configurations
    particles.resize(particle_count);

    lua_getfield(L, -1, "particles");

    for (int i = 1; i <= particle_count; i++) {
        lua_rawgeti(L, -1, i);

        Particle p = particles[i - 1];

        if (lua_istable(L, -1)) {
            GET_PARTICLE_FIELD(L, "x", p.x, lua_tonumber);
            GET_PARTICLE_FIELD(L, "y", p.y, lua_tonumber);
            GET_PARTICLE_FIELD(L, "vx", p.vx, lua_tonumber);
            GET_PARTICLE_FIELD(L, "vy", p.vy, lua_tonumber);
            GET_PARTICLE_FIELD(L, "mass", p.mass, lua_tonumber);
            GET_PARTICLE_FIELD(L, "isGravNode", p.is_grav_node, lua_tointeger);
            GET_PARTICLE_FIELD(L, "isMoveable", p.is_moveable, lua_tointeger);
        }

        lua_pop(L, 1);
    }

    lua_pop(L, 2);

    lua_close(L);

    // Initializing simulation
    ParticleSimulator simulator =
        ParticleSimulator(particles.data(), particle_count, gravity, max_speed,
                          screen_width, screen_height, target_fps, show_debug_info,
                          simulation_threads_per_block, renderer_threads_per_block);
    simulator.run();

    return 0;
}
