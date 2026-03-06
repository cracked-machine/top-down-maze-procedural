#ifndef SRC_FACTORY_OBSTACLEFACTORY_HPP
#define SRC_FACTORY_OBSTACLEFACTORY_HPP

#include <Components/Inventory/CarryItem.hpp>
#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/fwd.hpp>

namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Factory
{

//! @brief Create a Obstacle component at the given entt
//! @param registry
//! @param entity
//! @param pos_cmp
//! @param sprite_type
//! @param sprite_tile_idx
//! @param zorder
void create_obstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                      std::size_t sprite_tile_idx, float zorder );

//! @brief Remove the obstacle component from the entity
//! @param registry
//! @param obstacle_entity
void remove_obstacle( entt::registry &registry, entt::entity obstacle_entity );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP