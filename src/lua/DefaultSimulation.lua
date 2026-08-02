simulation.gravity = 9.81
simulation.maxSpeed = 300.0
simulation.screenWidth = 720
simulation.screenHeight = 900
simulation.targetFPS = 120
simulation.showDebugInfo = true
simulation.particleCount = 16384

local centerX = simulation.screenWidth / 2
local centerY = simulation.screenHeight / 2

if simulation.particleCount > 0 then
    simulation.particles[1] = {
        x = centerX,
        y = centerY,
        vx = 0.0,
        vy = 0.0,
        mass = 500000.0,
        isGravNode = 1,
        isMoveable = 0
    }
end

local minRadius = 60.0
local maxRadius = 280.0

for i = 2, simulation.particleCount do
    local angle = math.random() * (2.0 * math.pi)
    
    local radius = minRadius + math.random() * (maxRadius - minRadius)

    local x = centerX + radius * math.cos(angle)
    local y = centerY + radius * math.sin(angle)

    local orbitalSpeed = math.sqrt(5000000.0 / radius)

    local vx = orbitalSpeed * math.sin(angle)
    local vy = -orbitalSpeed * math.cos(angle)

    simulation.particles[i] = {
        x = x,
        y = y,
        vx = vx,
        vy = vy,
        mass = 1.0,
        isGravNode = 0,
        isMoveable = 1
    }
end
