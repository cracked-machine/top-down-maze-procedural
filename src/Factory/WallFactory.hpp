#ifndef SRC_FACTORY_WALLFACTORY_HPP_
#define SRC_FACTORY_WALLFACTORY_HPP_

#include <SFML/System/Vector2.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Factory
{

void add_wall_entity( const sf::Vector2f &pos, Sprites::SpriteMetaType sprite_type, std::size_t sprite_index );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_WALLFACTORY_HPP_