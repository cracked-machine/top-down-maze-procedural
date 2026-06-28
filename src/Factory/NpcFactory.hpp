#ifndef SRC_FACTORY_NPCFACTORY_HPP
#define SRC_FACTORY_NPCFACTORY_HPP

#include <Components/Position.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/fwd.hpp>

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

void create_npc_container( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                           std::size_t sprite_tile_idx, float zorder );
void destroy_npc_container( entt::registry &registry, entt::entity npc_container_entity );

//! @brief Create a new NPC. Creates a new entity at `position_entity`. Returns the new entity.
//! @param registry
//! @param position_entity
//! @param npc_type
//! @return entt::entity
entt::entity create_npc( entt::registry &registry, entt::entity position_entity, const std::string &npc_type );
entt::entity destroy_npc( entt::registry &registry, entt::entity npc_entity );

entt::entity create_npc_explosion( entt::registry &registry, Cmp::Position npc_position_cmp );
void remove_npc_explosion( entt::registry &registry, entt::entity entity );

bool create_shockwave( entt::registry &registry, entt::entity npc_entt );

// Iterate and generate npc containers
std::vector<entt::entity> gen_npc_containers( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size );

} // namespace Game::Factory

#endif // SRC_FACTORY_NPCFACTORY_HPP