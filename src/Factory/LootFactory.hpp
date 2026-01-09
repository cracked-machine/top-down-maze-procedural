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

namespace detail
{
template <typename T>
bool hasComponentAtPosition( entt::registry &registry, const Cmp::Position &pos )
{
  for ( auto [entt, cmp, cmp_pos] : registry.view<T, Cmp::Position>().each() )
  {
    if ( pos.findIntersection( cmp_pos ) ) return true;
  }
  return false;
}

template <typename... SpatialExclude>
bool hasAnySpatialExcludedAtPosition( entt::registry &registry, const Cmp::Position &pos, ExcludePack<SpatialExclude...> )
{
  return ( hasComponentAtPosition<SpatialExclude>( registry, pos ) || ... );
}
} // namespace detail

/**
 * @brief Attempts to create a loot drop at a traversable position within a specified search area.
 *
 * @tparam Include Component types the position entity must have.
 * @tparam Exclude Component types the position entity must NOT have.
 * @tparam SpatialExclude Component types to check spatially (separate entities at same position).
 * @param registry The entity registry.
 * @param loot_anim_cmp The sprite animation for the loot.
 * @param search The area in which to search for a suitable position.
 * @param zorder_offset Offset added to position.y for z-ordering.
 * @return The newly created loot entity, or entt::null if no suitable location was found.
 */
template <typename... Include, typename... Exclude, typename... SpatialExclude>
inline entt::entity createLootDrop( entt::registry &registry, Cmp::SpriteAnimation &&loot_anim_cmp, sf::FloatRect search, IncludePack<Include...>,
                                    ExcludePack<Exclude...>, ExcludePack<SpatialExclude...> spatial_exclude = ExcludePack<>{},
                                    float zorder_offset = -8.f )
{
  auto pos_view = registry.view<Cmp::Position, Include...>( entt::exclude<Exclude...> );

  for ( auto [pos_entity, pos_cmp] : pos_view.each() )
  {
    if ( not search.findIntersection( pos_cmp ) ) continue;

    // Skip non-traversable obstacles
    auto obst_cmp = registry.try_get<Cmp::Obstacle>( pos_entity );

    // Skip positions occupied by spatially excluded entities
    if ( detail::hasAnySpatialExcludedAtPosition( registry, pos_cmp, spatial_exclude ) ) continue;

    auto new_loot_entity = registry.create();
    registry.emplace<Cmp::Position>( new_loot_entity, pos_cmp.position, pos_cmp.size );
    registry.emplace<Cmp::SpriteAnimation>( new_loot_entity, loot_anim_cmp );
    registry.emplace<Cmp::ZOrderValue>( new_loot_entity, pos_cmp.position.y + zorder_offset );
    registry.emplace<Cmp::Loot>( new_loot_entity );
    SPDLOG_INFO( "Created loot entity {} of type {} at position ({}, {})", static_cast<int>( new_loot_entity ), loot_anim_cmp.m_sprite_type,
                 pos_cmp.position.x, pos_cmp.position.y );
    return new_loot_entity;
  }

  SPDLOG_WARN( "Failed to drop {} at [{},{}].", loot_anim_cmp.m_sprite_type, search.position.x, search.position.y );
  return entt::null;
}

void destroyLootDrop( entt::registry &registry, entt::entity loot_entity );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_LOOTFACTORY_HPP__