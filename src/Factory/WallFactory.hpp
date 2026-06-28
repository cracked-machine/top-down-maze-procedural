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

namespace Game::Factory
{

void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, const Sprites::SpriteSheet &ms, std::size_t sprite_index );
void add_reservedposition( entt::registry &reg, const sf::Vector2f &pos );
void add_solid_player( entt::registry &reg, sf::FloatRect rect );
void add_solid_npc( entt::registry &reg, sf::FloatRect rect );
void add_no_move_dest( entt::registry &reg, sf::FloatRect rect );
} // namespace Game::Factory

#endif // SRC_FACTORY_WALLFACTORY_HPP__
