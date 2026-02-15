#ifndef SRC_UTILS_REGISTRY_HPP_
#define SRC_UTILS_REGISTRY_HPP_

#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Utils::Collision
{

//! @brief Check if `Component` position collides with another component
//! @tparam Component Associated entity must also own a Cmp::Position
//! @param reg reference to the entt reg
//! @param pos Does this position contain a `Component`
//! @param filter Optional callback to filter candidates. Return true to consider this entity, false to skip.
//! @return requires
template <typename Component>
bool check_cmp( entt::registry &reg, Cmp::RectBounds pos, std::function<bool( const Component & )> filter = []( const Component & ) { return true; } )
{
  for ( auto [candidate_entt, candidate_cmp, candidate_pos] : reg.view<Component, Cmp::Position>().each() )
  {
    if ( not filter( candidate_cmp ) ) continue;
    if ( pos.findIntersection( candidate_pos ) ) { return true; }
  }
  return false;
};

// Concept to check if Component inherits from one of the valid position/bounds types
template <typename T>
concept HasPositionBounds = std::is_base_of_v<Cmp::Position, T> || std::is_base_of_v<Cmp::RectBounds, T> || std::is_base_of_v<sf::FloatRect, T>;

//! @brief Check if a `Component` collides with another component type
//! @tparam `Component` must inherit from Cmp::Position, Cmp::RectBounds, sf::FloatRect
//! @param reg reference to the entt reg
//! @param pos Does this position contain a `Component`
//! @param filter Optional callback to filter candidates. Return true to consider this entity, false to skip.
//! @return requires
template <typename Component>
  requires HasPositionBounds<Component>
bool check_pos( entt::registry &reg, Cmp::RectBounds pos, std::function<bool( const Component & )> filter = []( const Component & ) { return true; } )
{
  for ( auto [candidate_entt, candidate_cmp] : reg.view<Component>().each() )
  {
    if ( not filter( candidate_cmp ) ) continue;
    if ( pos.findIntersection( candidate_cmp ) ) { return true; }
  }
  return false;
}

} // namespace ProceduralMaze::Utils::Collision

#endif // SRC_UTILS_REGISTRY_HPP_