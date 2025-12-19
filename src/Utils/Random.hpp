#ifndef SRC_UTILS_RANDOM_HPP__
#define SRC_UTILS_RANDOM_HPP__

#include <Components/CryptRoomClosed.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cstddef>
#include <entt/entity/registry.hpp>
#include <set>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Utils::Rnd
{

// Helper structs for variadic template parameter packs
template <typename... Types>
struct IncludePack
{
};

// Helper structs for variadic template parameter packs
template <typename... Types>
struct ExcludePack
{
};

/**
 * @brief Retrieves a random entity and its position component from entities matching the
 * specified criteria.
 *
 * This function selects a random entity and position component from a filtered view of entities
 * that have a Position component and all specified Include components, while excluding entities
 * with any of the Exclude components.
 *
 * @tparam Include... Variadic template parameter pack specifying component types that entities
 * must have
 * @tparam Exclude... Variadic template parameter pack specifying component types that entities
 * must not have
 *
 * @param include_pack Template parameter pack wrapper for components to include in the filter. Do
 * not include Cmp::Position here.
 * @param exclude_pack Template parameter pack wrapper for components to exclude from the filter
 * @param seed Optional seed value for random number generation. If 0 (default), uses
 * std::random_device
 *
 * @return std::pair<entt::entity, Cmp::Position> A pair containing the randomly selected entity
 * and its position component
 *
 * @note The function assumes there is at least one entity matching the filter criteria.
 *       If no entities match, the behavior is undefined.
 * @note Uses SPDLOG_DEBUG to log the number of matching positions found.
 */
template <typename... Include, typename... Exclude>
static std::pair<entt::entity, Cmp::Position> get_random_position( entt::registry &reg, IncludePack<Include...>, ExcludePack<Exclude...>,
                                                                   unsigned long seed = 0 )
{
  auto random_view = reg.view<Cmp::Position, Include...>( entt::exclude<Exclude...> );

  auto random_view_count = std::distance( random_view.begin(), random_view.end() );

  SPDLOG_DEBUG( "Found {} positions in the maze.", random_view_count );

  // Get random index and advance iterator to that position
  Cmp::RandomInt seed_picker( 0, static_cast<int>( random_view_count - 1 ) );
  if ( seed != 0 ) { seed_picker.seed( seed ); }
  else { seed_picker.seed( std::random_device{}() ); }

  int random_index = seed_picker.gen();
  SPDLOG_DEBUG( "Random index selected: {}", random_index );
  if ( random_index < 0 || random_index >= random_view_count )
  {
    SPDLOG_CRITICAL( "Random index {} out of bounds (0 to {})", random_index, random_view_count - 1 );
    throw std::out_of_range( "Random index out of bounds" );
  }
  auto it = random_view.begin();
  std::advance( it, random_index );

  // Get the random entity
  entt::entity random_entity = *it;

  // Get the position component
  Cmp::Position random_position = random_view.template get<Cmp::Position>( random_entity );

  return { random_entity, random_position };
}

//! @brief Get the n random components of a specific type from the registry
//! @note If result_size > 70% of the sample space then this function may be inefficient.
//!       See "coupon collector" problem for more details
//! @tparam Component The type of component to retrieve
//! @tparam Include Additional types of components that must be included
//! @tparam Exclude Additional types of components that must be excluded
//! @param reg The entity registry
//! @param result_size The number of random components to retrieve
//! @param seed Optional seed value for random number generation
//! @return std::map<entt::entity, Component> A map of entities to their corresponding components
template <typename Component, typename... Include, typename... Exclude>
static std::set<entt::entity> get_n_rand_components( entt::registry &reg, std::size_t result_size, IncludePack<Include...>, ExcludePack<Exclude...>,
                                                     unsigned long seed = 0 )
{
  std::set<entt::entity> results;
  if ( result_size == 0 )
  {
    SPDLOG_WARN( "Requested 0 random components. Returning empty result set." );
    return results;
  }

  auto cmp_view = reg.view<Component, Include...>( entt::exclude<Exclude...> );
  auto cmp_view_size = std::distance( cmp_view.begin(), cmp_view.end() );
  SPDLOG_DEBUG( "Found {} components in the maze.", cmp_view_size );

  // clamp result size if there are fewer components available
  if ( cmp_view_size < result_size )
  {
    SPDLOG_WARN( "Requested {} random components, but only {} available. Returning all available components.", result_size, cmp_view_size );
    result_size = cmp_view_size;
  }
  if ( cmp_view_size == result_size )
  {
    // If the number of available entities matches the requested size, return all of them
    for ( auto entity : cmp_view )
    {
      results.insert( entity );
    }
    return results;
  }

  // Set seed for random generator
  Cmp::RandomInt seed_picker( 0, static_cast<int>( cmp_view_size - 1 ) );
  if ( seed != 0 ) { seed_picker.seed( seed ); }
  else { seed_picker.seed( std::random_device{}() ); }

  // keep picking random components until we reach the desired result size of unique components
  while ( results.size() < result_size )
  {
    // pick random index and advance iterator to that position
    int random_index = std::clamp( seed_picker.gen(), 0, static_cast<int>( cmp_view_size - 1 ) );
    auto it = cmp_view.begin();
    std::advance( it, random_index );

    // Get the random entity and position component
    entt::entity random_entity = *it;
    if ( results.find( random_entity ) != results.end() ) continue; // already have this one

    results.insert( random_entity );
  }
  return results;
}

template <typename... Include, typename... Exclude>
entt::entity get_random_nearby_disabled_obstacle( entt::registry &reg, sf::FloatRect search_area, IncludePack<Include...>, ExcludePack<Exclude...> )
{

  auto obst_view = reg.view<Cmp::Obstacle, Cmp::Position, Include...>( entt::exclude<Exclude...> );

  for ( auto obst_entity : obst_view )
  {
    auto &obst_cmp = obst_view.template get<Cmp::Obstacle>( obst_entity );
    auto &obst_pos_cmp = obst_view.template get<Cmp::Position>( obst_entity );
    if ( obst_cmp.m_enabled ) continue; // only drop the loot at disabled (traversable) obstacle
    if ( not search_area.findIntersection( obst_pos_cmp ) ) continue;

    return obst_entity;
  }
  SPDLOG_WARN( "Failed to find nearby disabled obstacle for [{},{}].", search_area.position.x, search_area.position.y );
  return entt::null;
}
} // namespace ProceduralMaze::Utils::Rnd

#endif // SRC_UTILS_RANDOM_HPP__