#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry );
entt::entity createSpawnPointMarker( entt::registry &registry, const sf::Vector2f &pos,
                                     Cmp::RectBounds player_start_area, float zorder );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP