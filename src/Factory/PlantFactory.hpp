#ifndef SRC_FACTORY_PLANTFACTORY_HPP_
#define SRC_FACTORY_PLANTFACTORY_HPP_

#include <Components/Position.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Sprites
{
class SpriteFactory;

} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Factory
{
entt::entity create_plant_obstacle( entt::registry &reg, Cmp::Position pos_cmp, const Sprites::MultiSprite &ms );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLANTFACTORY_HPP_