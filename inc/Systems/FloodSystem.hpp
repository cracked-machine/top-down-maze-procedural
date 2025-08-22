#ifndef __SYSTEMS_WATER_SYSTEM_HPP__
#define __SYSTEMS_WATER_SYSTEM_HPP__

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
        if( m_clock.getElapsedTime() > sf::seconds(dt) )
        {
            for(auto [entity, _wl]: m_reg->view<Cmp::WaterLevel>().each()) 
            {
                
                _wl.m_level -= (dt * m_flood_velocity);
                SPDLOG_TRACE("Updating flood water levels to: {}", _wl.m_level);
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