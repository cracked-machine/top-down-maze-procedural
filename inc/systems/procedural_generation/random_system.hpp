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
        const sf::Vector2u &size,
        entt::basic_registry<entt::entity> &reg,
        const sf::Vector2f &offset = sf::Vector2f{0,0}
    )
        : m_size(size)
    {
        using entity_trait = entt::entt_traits<entt::entity>;
        for(int x = 0; x < m_size.x; x++)
        {
            for(int y = 0; y < m_size.y; y++)
            {
                auto entity = reg.create();
                reg.emplace<Cmp::Position>( 
                    entity, 
                    sf::Vector2f{
                        x * (Sprites::Brick::WIDTH + Sprites::Brick::LINEWIDTH)  + offset.x, 
                        y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH) + offset.y
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

protected:
    std::vector<uint32_t> m_neighbourhood;
    const sf::Vector2u m_size;
private:
    Cmp::Random rng{0, 1};
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__