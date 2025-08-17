#ifndef __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <obstacle.hpp>
#include <position.hpp>
#include <spdlog/spdlog.h>

#include <sprites/brick.hpp>
#include <components/random.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class RandomSystem {
public:
    RandomSystem(
        const sf::Vector2u &grid_size,
        const sf::Vector2f &grid_offset = sf::Vector2f{0,0}
    )
    : 
        m_grid_size(grid_size),
        m_grid_offset(grid_offset)
    {
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

                m_neighbourhood.push_back(entity_trait::to_entity(entity));
                if(rng.gen())
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

    uint32_t get_nb(std::size_t idx) { return m_neighbourhood.at(idx); }
    auto begin() { return m_neighbourhood.begin(); }
    auto end() { return m_neighbourhood.end(); }
    auto size() { return m_neighbourhood.size(); }
    
    const sf::Vector2u m_grid_size;
    const sf::Vector2f m_grid_offset;
private:
    std::vector<uint32_t> m_neighbourhood;
    Cmp::Random rng{0, 1};
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__