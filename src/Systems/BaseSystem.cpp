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
    if ( obs_hitbox.findIntersection( target_hitbox ) ) return false;
  }
  return true;
}

} // namespace ProceduralMaze::Sys