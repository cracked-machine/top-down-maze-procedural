#ifndef __SYSTEMS_WATER_SYSTEM_HPP__
#define __SYSTEMS_WATER_SYSTEM_HPP__

#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <WaterLevel.hpp>
#include <entt/entity/registry.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class FloodSystem {
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

    void update() 
    {   
        // update the water level is timer has elapsed
        if( m_clock.getElapsedTime() > sf::seconds(dt) )
        {
            for(auto [entity, _wl]: m_reg->view<Cmp::WaterLevel>().each()) 
            {
                _wl.m_level -= (dt * m_flood_velocity);                
                SPDLOG_INFO("Updating flood water levels to: {}", _wl.m_level);

                // is player drowning
                for(auto [_, _pc, _pos]: m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each()) 
                {
                    if ( _wl.m_level < (_pos.y - 16) )
                    {
                        _pc.health -= 5;
                        SPDLOG_INFO("Updating playable character health to: {}", _pc.health);  
                    }
                    if (_pc.health == 0) {
                        _pc.alive = false;
                        SPDLOG_INFO("Player drowned!");

                    }
                }

                m_clock.restart();
            }
        }
    }

private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    sf::Clock m_clock;
    float dt = 1.0f;
    float m_flood_velocity; // pixels per second

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_WATER_SYSTEM_HPP__