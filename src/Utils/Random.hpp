#ifndef SRC_UTILS_RANDOM_HPP__
#define SRC_UTILS_RANDOM_HPP__

#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <entt/entity/registry.hpp>
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
static std::pair<entt::entity, Cmp::Position> get_random_position( entt::registry &reg, IncludePack<Include...>,
                                                                   ExcludePack<Exclude...>, unsigned long seed = 0 )
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

template <typename... Include, typename... Exclude>
entt::entity get_random_nearby_disabled_obstacle( entt::registry &reg, sf::FloatRect search_area,
                                                  IncludePack<Include...>, ExcludePack<Exclude...> )
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