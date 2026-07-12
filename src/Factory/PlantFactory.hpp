#ifndef SRC_FACTORY_PLANTFACTORY_HPP__
#define SRC_FACTORY_PLANTFACTORY_HPP__

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
//! @brief Remove the components of plant multiblock and segment entities
//! @param reg
//! @param plant_entt
void remove_plant_mb( entt::registry &reg, entt::entity plant_entt );

} // namespace Game::Factory

#endif // SRC_FACTORY_PLANTFACTORY_HPP__
