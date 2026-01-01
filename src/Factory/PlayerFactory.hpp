#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry );
entt::entity createWorldPosition( entt::registry &registry, const sf::Vector2f &pos );
entt::entity createVoidPosition( entt::registry &registry, const sf::Vector2f &pos );
void addSpawnArea( entt::registry &registry, entt::entity entity, float zorder );
void createPlayerExplosion( entt::registry &registry, Cmp::Position npc_pos_cmp );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP