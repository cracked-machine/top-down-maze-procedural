#ifndef SRC_FACTORY_NPCFACTORY_HPP
#define SRC_FACTORY_NPCFACTORY_HPP

#include <Components/Position.hpp>
#include <Sprites/MultiSprite.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

void createNpcContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                         std::size_t sprite_tile_idx, float zorder );
void destroyNpcContainer( entt::registry &registry, entt::entity npc_container_entity );
void createNPC( entt::registry &registry, entt::entity position_entity, const std::string &type );
entt::entity destroyNPC( entt::registry &registry, entt::entity npc_entity );
void createNpcExplosion( entt::registry &registry, Cmp::Position npc_position_cmp );
} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_NPCFACTORY_HPP