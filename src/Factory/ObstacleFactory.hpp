#ifndef SRC_FACTORY_OBSTACLEFACTORY_HPP
#define SRC_FACTORY_OBSTACLEFACTORY_HPP

#include <Components/Position.hpp>
#include <Sprites/MultiSprite.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{
void createObstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                     std::size_t sprite_tile_idx, float zorder );

void destroyObstacle( entt::registry &registry, entt::entity obstacle_entity );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP