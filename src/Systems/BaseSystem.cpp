#include <Door.hpp>
#include <Exit.hpp>
#include <ReservedPosition.hpp>
#include <Systems/BaseSystem.hpp>
#include <Wall.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys {

BaseSystem::BaseSystem( ProceduralMaze::SharedEnttRegistry reg )
    : m_reg( reg )
{
}

const sf::Vector2u BaseSystem::kDisplaySize{ 1920, 1024 };

const sf::Vector2u BaseSystem::kMapGridSize{ 100u, 124u };

const sf::Vector2f BaseSystem::kMapGridOffset{ 10.f, 1.f };

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_event_dispatcher = nullptr;

bool BaseSystem::is_valid_move( const sf::Vector2f &target_position )
{
  auto target_hitbox = get_hitbox( target_position );

  // Check obstacles
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obs_cmp, pos_cmp] : obstacle_view.each() )
  {
    if ( obs_cmp.m_enabled == false || obs_cmp.m_integrity <= 0.0f ) continue;
    auto obs_hitbox = get_hitbox( pos_cmp );
    if ( obs_hitbox.findIntersection( target_hitbox ) ) { return false; }
  }

  // Check walls
  auto wall_view = m_reg->view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {
    auto wall_hitbox = get_hitbox( pos_cmp );
    if ( wall_hitbox.findIntersection( target_hitbox ) ) { return false; }
  }

  // Check doors (excluding exits)
  auto door_view = m_reg->view<Cmp::Door, Cmp::Position>( entt::exclude<Cmp::Exit> );
  for ( auto [entity, door_cmp, pos_cmp] : door_view.each() )
  {
    auto door_hitbox = get_hitbox( pos_cmp );
    if ( door_hitbox.findIntersection( target_hitbox ) ) { return false; }
  }

  // Check reserved positions (excluding exits)
  auto reserved_view = m_reg->view<Cmp::ReservedPosition>();
  for ( auto [entity, reserved_cmp] : reserved_view.each() )
  {
    if ( not reserved_cmp.m_solid_mask ) continue;
    auto reserved_hitbox = get_hitbox( reserved_cmp );
    if ( reserved_hitbox.findIntersection( target_hitbox ) ) { return false; }
  }

  return true;
}

bool BaseSystem::isDiagonalMovementBetweenObstacles( const sf::Vector2f &current_pos, const sf::Vector2f &direction )
{
  if ( !( ( direction.x != 0.0f ) && ( direction.y != 0.0f ) ) ) return false; // Not diagonal

  float grid_size = BaseSystem::kGridSquareSizePixels.x;

  // Calculate the two orthogonal positions the diagonal movement would "cut through"
  sf::Vector2f horizontal_check = sf::Vector2f{ current_pos.x + ( direction.x * grid_size ), current_pos.y };

  sf::Vector2f vertical_check = sf::Vector2f{ current_pos.x, current_pos.y + ( direction.y * grid_size ) };

  // Check if both orthogonal positions have obstacles
  bool horizontal_blocked = !is_valid_move( horizontal_check );
  bool vertical_blocked = !is_valid_move( vertical_check );

  // If both orthogonal paths are blocked, diagonal movement is between obstacles
  return horizontal_blocked && vertical_blocked;
}

sf::FloatRect BaseSystem::calculate_view_bounds( const sf::View &view ) const
{
  // Calculate view bounds in world coordinates
  return sf::FloatRect( view.getCenter() - view.getSize() / 2.f, view.getSize() );
}

bool BaseSystem::is_visible_in_view( const sf::FloatRect &viewbounds, const sf::Vector2f &position ) const
{
  // Get the hitbox for the position
  auto hitbox = get_hitbox( position );

  // Check for intersection
  return viewbounds.findIntersection( hitbox ).has_value();
}

bool BaseSystem::is_visible_in_view( const sf::View &view, const sf::Vector2f &position ) const
{
  auto viewBounds = calculate_view_bounds( view );
  // Get the hitbox for the position
  auto hitbox = get_hitbox( position );

  // Check for intersection
  return viewBounds.findIntersection( hitbox ).has_value();
}

} // namespace ProceduralMaze::Sys