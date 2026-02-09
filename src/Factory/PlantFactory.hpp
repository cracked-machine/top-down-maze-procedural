#ifndef SRC_FACTORY_PLANTFACTORY_HPP_
#define SRC_FACTORY_PLANTFACTORY_HPP_

#include <Components/Position.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Sprites
{
class SpriteFactory;

} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Factory
{
entt::entity createPlantObstacle( entt::registry &reg, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type, float zorder );

void gen_random_plants( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLANTFACTORY_HPP_