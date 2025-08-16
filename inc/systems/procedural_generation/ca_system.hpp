#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
#include <obstacle.hpp>
#include <position.hpp>
#include <procedural_generation/random_system.hpp>
#include <spdlog/spdlog.h>

#include <sprites/brick.hpp>
#include <components/random.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class CellAutomataSystem : public RandomSystem {
public:
    CellAutomataSystem(
        const sf::Vector2u &size,
        entt::basic_registry<entt::entity> &reg,
        const sf::Vector2f &offset = sf::Vector2f{0,0}
    )
    :
        RandomSystem(size, reg, offset)
    {
        for( auto [_entt, _ob, _pos]: reg.view<Cmp::Obstacle, Cmp::Position>().each() ) {
            if( _ob.m_enabled )
            {

            }
        }
    }

    ~CellAutomataSystem() = default;

private:
    Cmp::Random rng{0, 1};
};

} // namespace ProceduralMaze::Systems::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__