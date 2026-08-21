#ifndef SRC_FACTORY_WALLFACTORY_HPP__
#define SRC_FACTORY_WALLFACTORY_HPP__
#include <entt/entity/fwd.hpp>

#include <SFML/System/Vector2.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

namespace Game::Factory::Wall
{

//! @brief Create a wall entity: reserved, sprite-decorated, and blocking pathfinding.
//! @param reg
//! @param pos
//! @param ms
//! @param sprite_index
void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, const Sprites::SpriteSheet &ms, std::size_t sprite_index );

//! @brief Create a bare reserved-position entity, without any sprite or wall component.
//! @param reg
//! @param pos
void add_reservedposition( entt::registry &reg, const sf::Vector2f &pos );

//! @brief Mark every existing world position entity overlapping `rect` as reserved and blocking player movement.
//! @param reg
//! @param rect
void add_solid_player( entt::registry &reg, sf::FloatRect rect );

//! @brief Mark every existing world position entity overlapping `rect` as reserved and blocking NPC pathfinding.
//! @param reg
//! @param rect
void add_solid_npc( entt::registry &reg, sf::FloatRect rect );

//! @brief Mark every existing world position entity overlapping `rect` as reserved and an invalid movement destination.
//! @param reg
//! @param rect
void add_no_move_dest( entt::registry &reg, sf::FloatRect rect );
} // namespace Game::Factory::Wall

#endif // SRC_FACTORY_WALLFACTORY_HPP__
