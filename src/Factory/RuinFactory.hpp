#ifndef SRC_FACTORY_RUINFACTORY_HPP_
#define SRC_FACTORY_RUINFACTORY_HPP_

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

#include <Sprites/SpriteMetaType.hpp>
namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Factory
{

// void create_staircase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms );
void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &bookcase_ms, int sprite_index );
void create_cobweb( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &cobweb_ms, int sprite_index );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_RUINFACTORY_HPP_