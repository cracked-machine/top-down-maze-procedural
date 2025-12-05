#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Position.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP