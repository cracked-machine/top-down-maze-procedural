#ifndef SRC_FACTORY_OBSTACLEFACTORY_HPP
#define SRC_FACTORY_OBSTACLEFACTORY_HPP

#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/fwd.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

namespace Game::Factory
{

entt::entity create_world_pos( entt::registry &registry, const sf::Vector2f &pos );
entt::entity create_void_pos( entt::registry &registry, const Cmp::Position &pos );

//! @brief Create an obstacle WITHOUT sprite for procedural generation algorithm
//! @param registry
//! @param entity
//! @param pos_cmp
//! @param ms
//! @param sprite_tile_idx
void add_obstacle( entt::registry &registry, entt::entity entity );

//! @brief Create an obstacle WITH sprite for final level gen pass
//! @param registry
//! @param entity
//! @param pos_cmp
//! @param ms
//! @param sprite_tile_idx
//! @param zorder
//! @param blocking
void decorate_obstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms,
                        std::size_t sprite_tile_idx, float zorder = 0, bool blocking = true );

//! @brief Remove the obstacle component from the entity
//! @param registry
//! @param obstacle_entity
//! @param delete_extras Delete any extra obstacles entitys found (doesn't delete the search_entt)
void remove_obstacle( entt::registry &reg, entt::entity search_entt, bool delete_extras = false );

} // namespace Game::Factory

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP