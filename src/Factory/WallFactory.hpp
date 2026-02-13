#ifndef SRC_FACTORY_WALLFACTORY_HPP_
#define SRC_FACTORY_WALLFACTORY_HPP_

#include <SFML/System/Vector2.hpp>
#include <Sprites/MultiSprite.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

enum class SolidWall { FALSE = false, TRUE = true };
void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, Sprites::SpriteMetaType sprite_type, std::size_t sprite_index, float zorder,
                      SolidWall solid_wall = SolidWall::TRUE );
void add_nopathfinding( entt::registry &reg, const sf::Vector2f &pos );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_WALLFACTORY_HPP_