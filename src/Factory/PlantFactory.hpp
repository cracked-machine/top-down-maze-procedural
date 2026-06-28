#ifndef SRC_FACTORY_PLANTFACTORY_HPP_
#define SRC_FACTORY_PLANTFACTORY_HPP_

#include <Components/Position.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

namespace Game::Sprites
{
class SpriteFactory;

} // namespace Game::Sprites

namespace Game::Factory
{
entt::entity create_plant_obstacle( entt::registry &reg, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms );

} // namespace Game::Factory

#endif // SRC_FACTORY_PLANTFACTORY_HPP_