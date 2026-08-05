/*
 * Copyright (c) 2026 LJC
 *
 * SPDX-License-Identifier: MIT
 */

// I could not just us a lua file so the preset lua scripts are
// just macros in a header file
// :P

#define DEFAULT_SIMULATION                                                     \
    R"(
simulation.gravity = 9.81
simulation.max_speed = 300.0
simulation.screen_width = 720
simulation.screen_height = 900
simulation.target_fps = 120
simulation.show_debug_info = true
simulation.particle_count = 16384
simulation.threads_per_block = 256
simulation.renderer_threads_per_block = 256

local center_x = simulation.screen_width / 2
local center_y = simulation.screen_height / 2

if simulation.particle_count > 0 then
    simulation.particles[1] = {
        x = center_x,
        y = center_y,
        vx = 0.0,
        vy = 0.0,
        mass = 500000.0,
        is_grav_node = 1,
        is_moveable = 0
    }
end

local min_radius = 60.0
local max_radius = 280.0

for i = 2, simulation.particle_count do
    local angle = math.random() * (2.0 * math.pi)
    
    local radius = min_radius + math.random() * (max_radius - min_radius)

    local x = center_x + radius * math.cos(angle)
    local y = center_y + radius * math.sin(angle)

    local orbital_speed = math.sqrt(5000000.0 / radius)

    local vx = orbital_speed * math.sin(angle)
    local vy = -orbital_speed * math.cos(angle)

    simulation.particles[i] = {
        x = x,
        y = y,
        vx = vx,
        vy = vy,
        mass = 1.0,
        is_grav_node = 0,
        is_moveable = 1
    }
end
)"
