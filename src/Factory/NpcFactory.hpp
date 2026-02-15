#ifndef SRC_FACTORY_NPCFACTORY_HPP
#define SRC_FACTORY_NPCFACTORY_HPP

#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/fwd.hpp>

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

void createNpcContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                         std::size_t sprite_tile_idx, float zorder );
void destroyNpcContainer( entt::registry &registry, entt::entity npc_container_entity );
void createNPC( entt::registry &registry, entt::entity position_entity, const std::string &type );
entt::entity destroyNPC( entt::registry &registry, entt::entity npc_entity );
void createNpcExplosion( entt::registry &registry, Cmp::Position npc_position_cmp );
void createShockwave( entt::registry &registry, entt::entity npc_entt );

// Iterate and generate npc containers
void gen_npc_containers( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_NPCFACTORY_HPP