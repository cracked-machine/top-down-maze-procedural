#ifndef SRC_FACTORY_WALLFACTORY_HPP_
#define SRC_FACTORY_WALLFACTORY_HPP_
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

enum class SolidWall { FALSE = false, TRUE = true };
void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, const Sprites::SpriteSheet &ms, std::size_t sprite_index,
                      SolidWall solid_wall = SolidWall::TRUE );
void add_nopathfinding( entt::registry &reg, const sf::Vector2f &pos );
void add_nonpcpathfinding( entt::registry &reg, const sf::Vector2f &pos );
void add_reservedposition( entt::registry &reg, const sf::Vector2f &pos );
entt::entity add_solid_player( entt::registry &reg, sf::FloatRect rect );
entt::entity add_solid_npc( entt::registry &reg, sf::FloatRect rect );
} // namespace Game::Factory

#endif // SRC_FACTORY_WALLFACTORY_HPP_