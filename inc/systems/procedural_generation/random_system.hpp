#ifndef __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
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
    {
        for(int x = 0; x < size.x; x++)
        {
            for(int y = 0; y < size.y; y++)
            {
                auto entity = reg.create();
                reg.emplace<Cmp::Position>( 
                    entity, 
                    sf::Vector2f{
                        x * (Sprites::Brick::WIDTH + Sprites::Brick::LINEWIDTH)  + offset.x, 
                        y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH) + offset.y
                    } 
                ); 

                if(rng.gen())
                {
                    reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BRICK, true, true );
                }
                else
                {
                    reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BRICK, false, false );
                }
            }
        }
    }

    ~RandomSystem() = default;

private:
    Cmp::Random rng{0, 1};
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_SYSTEM_HPP__