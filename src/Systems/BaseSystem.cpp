#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_event_dispatcher = nullptr;

bool BaseSystem::is_valid_move( sf::Vector2f &target_position )
{
  auto target_hitbox = get_hitbox( target_position );

  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obs_cmp, pos_cmp] : obstacle_view.each() )
  {
    if ( obs_cmp.m_enabled == false ) continue;
    auto obs_hitbox = get_hitbox( pos_cmp );
    if ( obs_hitbox.findIntersection( target_hitbox ) ) { return false; }
  }
  return true;
}

bool BaseSystem::isDiagonalMovementBetweenObstacles( const sf::Vector2f &current_pos, const sf::Vector2f &direction )
{
  if ( !( ( direction.x != 0.0f ) && ( direction.y != 0.0f ) ) ) return false; // Not diagonal

  float grid_size = Sprites::MultiSprite::kDefaultSpriteDimensions.x;

  // Calculate the two orthogonal positions the diagonal movement would "cut through"
  sf::Vector2f horizontal_check = sf::Vector2f{ current_pos.x + ( direction.x * grid_size ), current_pos.y };

  sf::Vector2f vertical_check = sf::Vector2f{ current_pos.x, current_pos.y + ( direction.y * grid_size ) };

  // Check if both orthogonal positions have obstacles
  bool horizontal_blocked = !is_valid_move( horizontal_check );
  bool vertical_blocked = !is_valid_move( vertical_check );

  // If both orthogonal paths are blocked, diagonal movement is between obstacles
  return horizontal_blocked && vertical_blocked;
}

} // namespace ProceduralMaze::Sys