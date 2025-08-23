#ifndef __SYSTEMS_WATER_SYSTEM_HPP__
#define __SYSTEMS_WATER_SYSTEM_HPP__

#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <WaterLevel.hpp>
#include <entt/entity/registry.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace ProceduralMaze::Sys {

class FloodSystem {
private:
    static constexpr float FIXED_TIMESTEP = 1.0f/30.0f; // Reduce to 30 FPS to decrease CPU load
    static constexpr float DAMAGE_COOLDOWN = 1.0f; // 1 second between damage applications
    float m_accumulator = 0.0f;
    sf::Clock m_clock;
    
    // Track last damage time for each player
    std::unordered_map<entt::entity, float> m_last_damage_time;

public:
    FloodSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        float flood_velocity
    ) :   
        m_reg(reg), m_flood_velocity(flood_velocity) 
    {
    }

    ~FloodSystem() = default;
    
    void add_flood_water_entity()
    {
        SPDLOG_INFO("Creating flood water entity");
        auto entity = m_reg->create();
        m_reg->emplace<Cmp::WaterLevel>(entity,
            Settings::DISPLAY_SIZE.y - 1               // initial level
        );
    }

    void update() {
        float frameTime = m_clock.restart().asSeconds();
        
        // Clamp frame time to prevent spiral of death
        frameTime = std::min(frameTime, 0.25f);
        
        m_accumulator += frameTime;
        
        // Process fixed timesteps
        while(m_accumulator >= FIXED_TIMESTEP) {
            updateFlood(FIXED_TIMESTEP);
            m_accumulator -= FIXED_TIMESTEP;
        }
    }
    
private:
    void updateFlood(float dt) {
        static float total_time = 0.0f;
        total_time += dt;
        
        // Cache views once - better performance since entities always exist
        auto water_view = m_reg->view<Cmp::WaterLevel>();
        auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
        
        // Separate water level updates from collision checks for better performance
        for(auto [_, water_level]: water_view.each()) {
            water_level.m_level -= (dt * m_flood_velocity);
        }
        
        // Check drowning - fixed logic: player drowns when water level is ABOVE player
        for(auto [_, water_level]: water_view.each()) {
            for(auto [player_entity, player_char, position]: player_view.each()) 
            {
                if(water_level.m_level <= (position.y - 16)) // Water drowns player when water level is at or above player position
                {
                    // Check if enough time has passed since last damage
                    auto it = m_last_damage_time.find(player_entity);
                    if(it == m_last_damage_time.end() || 
                       (total_time - it->second) >= DAMAGE_COOLDOWN) 
                    {
                        player_char.health -= 5;
                        SPDLOG_TRACE("player health {}", player_char.health);
                        m_last_damage_time[player_entity] = total_time;
                        
                        if(player_char.health <= 0) {
                            player_char.alive = false;
                            SPDLOG_TRACE("Player has drowned!");
                        }
                    }
                }
                else 
                {
                    // Player is out of water, remove from damage tracking
                    m_last_damage_time.erase(player_entity);
                }
            }
        }
    }

private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    float m_flood_velocity; // pixels per second
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_WATER_SYSTEM_HPP__