#ifndef SRC_FACTORY_ALTARFACTORY_HPP_
#define SRC_FACTORY_ALTARFACTORY_HPP_

#include <Components/Position.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

void createAltarSacrificeAnimation( entt::registry &reg, Cmp::Position pos );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_ALTARFACTORY_HPP_