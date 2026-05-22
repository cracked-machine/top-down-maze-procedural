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

//! @brief Create a obstacle object
//! @param registry
//! @param entity
//! @param pos_cmp
//! @param ms
//! @param sprite_tile_idx
void create_obstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms,
                      std::size_t sprite_tile_idx );

//! @brief Remove the obstacle component from the entity
//! @param registry
//! @param obstacle_entity
void remove_obstacle( entt::registry &registry, entt::entity obstacle_entity );

} // namespace Game::Factory

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP