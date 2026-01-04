#ifndef SRC_FACTORY_LOOTFACTORY_HPP__
#define SRC_FACTORY_LOOTFACTORY_HPP__

#include <Components/Loot.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/Factory.hpp>
#include <Systems/BaseSystem.hpp>

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

void createLootContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                          std::size_t sprite_tile_idx, float zorder );

void destroyLootContainer( entt::registry &registry, entt::entity loot_entity );

/**
 * @brief Attempts to create a loot drop at a traversable obstacle within a specified search area.
 *
 * Iterates over obstacles in the registry that match the given inclusion and exclusion criteria.
 * For each obstacle, checks if its position intersects with the search area and if it is disabled (traversable).
 * If a suitable obstacle is found, creates a new entity, assigns the provided loot component to it,
 * and places it at the obstacle's position.
 *
 * @tparam Include... Component types to include in the view.
 * @tparam Exclude... Component types to exclude from the view.
 * @param loot_cmp The loot component to be assigned to the new entity.
 * @param search The area in which to search for a suitable obstacle.
 * @param include_pack Pack of components to include in the view.
 * @param exclude_pack Pack of components to exclude from the view.
 * @return entt::entity The newly created loot entity, or entt::null if no suitable location was found.
 */
template <typename... Include, typename... Exclude>
inline entt::entity createLootDrop( entt::registry &registry, Cmp::SpriteAnimation &&loot_anim_cmp, sf::FloatRect search, IncludePack<Include...>,
                                    ExcludePack<Exclude...>, float zorder_offset = -8.f )
{
  entt::entity new_loot_entity = entt::null;
  auto pos_view = registry.view<Cmp::Position, Include...>( entt::exclude<Exclude...> );

  for ( auto [pos_entity, pos_cmp] : pos_view.each() )
  {
    if ( search.findIntersection( pos_cmp ) )
    {
      auto obst_cmp = registry.try_get<Cmp::Obstacle>( pos_entity );
      if ( obst_cmp and obst_cmp->m_enabled ) continue; // dont drop the loot at non-traversable obstacle

      new_loot_entity = registry.create();
      registry.emplace<Cmp::Position>( new_loot_entity, pos_cmp.position, pos_cmp.size );
      registry.emplace<Cmp::SpriteAnimation>( new_loot_entity, loot_anim_cmp );
      registry.emplace<Cmp::ZOrderValue>( new_loot_entity, pos_cmp.position.y + zorder_offset );
      registry.emplace<Cmp::Loot>( new_loot_entity );
      SPDLOG_INFO( "Created loot entity {} of type {} at position ({}, {})", static_cast<int>( new_loot_entity ), loot_anim_cmp.m_sprite_type,
                   pos_cmp.position.x, pos_cmp.position.y );
      return new_loot_entity;
    }
  }
  SPDLOG_WARN( "Failed to drop {} at [{},{}].", loot_anim_cmp.m_sprite_type, search.position.x, search.position.y );
  return new_loot_entity;
}

void destroyLootDrop( entt::registry &registry, entt::entity loot_entity );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_LOOTFACTORY_HPP__