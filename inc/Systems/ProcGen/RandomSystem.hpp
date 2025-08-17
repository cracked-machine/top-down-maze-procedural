#ifndef __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Sprites/Brick.hpp>

namespace ProceduralMaze::Sys::ProcGen {


class RandomSystem {
public:
    RandomSystem(
        const sf::Vector2u &grid_size,
        const sf::Vector2f &grid_offset = sf::Vector2f{0,0},
        const std::pair<unsigned int, unsigned int> &range = {0,1}
    )
    : 
        m_grid_size(grid_size),
        m_grid_offset(grid_offset),
        m_rng(range.first, range.second)
    {
    }

    void gen(unsigned long seed = 0)
    {
        if (seed) Cmp::Random::seed(seed);
        for(int x = 0; x < m_grid_size.x; x++)
        {
            for(int y = 0; y < m_grid_size.y; y++)
            {
                m_data.push_back(m_rng.gen());
            }
        }
    }

    void gen(entt::basic_registry<entt::entity> &reg, unsigned long seed = 0)
    {
        if (seed) Cmp::Random::seed(seed);
        using entity_trait = entt::entt_traits<entt::entity>;
        for(int x = 0; x < m_grid_size.x; x++)
        {
            for(int y = 0; y < m_grid_size.y; y++)
            {
                auto entity = reg.create();
                reg.emplace<Cmp::Position>( 
                    entity, 
                    sf::Vector2f{
                        x * (Sprites::Brick::WIDTH + Sprites::Brick::LINEWIDTH)  + m_grid_offset.x, 
                        y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH) + m_grid_offset.y
                    } 
                ); 

                m_data.push_back(entity_trait::to_entity(entity));
                if(m_rng.gen())
                {
                    reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BRICK, true, true );                    
                }
                else
                {
                    reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BRICK, true, false );
                }
            }
        }
    }

    ~RandomSystem() = default;

    std::optional<uint32_t> at(std::size_t idx) 
    { 
        if( idx > m_data.size() ) return std::nullopt ;
        else return m_data.at(idx); 
    }
    auto data() { return m_data.data(); }
    auto begin() { return m_data.begin(); }
    auto end() { return m_data.end(); }
    auto size() { return m_data.size(); }
    
    const sf::Vector2u m_grid_size;
    const sf::Vector2f m_grid_offset;
private:
    std::vector<uint32_t> m_data;
    Cmp::Random m_rng;
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__