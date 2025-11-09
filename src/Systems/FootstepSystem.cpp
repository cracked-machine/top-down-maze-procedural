#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/FootstepSystem.hpp>

namespace ProceduralMaze::Sys {

void FootstepSystem::onPause()
{
  auto footstep_timer_view = m_reg->view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // pause each footstep timer clock
    timer.m_clock.stop();
  }
}

void FootstepSystem::onResume()
{
  auto footstep_timer_view = m_reg->view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // resume each footstep timer clock
    timer.m_clock.start();
  }
}

void FootstepSystem::add_footstep( const Cmp::Position &pos_cmp, const Cmp::Direction &direction )
{
  if ( update_clock.getElapsedTime() >= kFootstepAddDelay )
  {
    // Check if position is inside a spawn area
    auto spawnarea_view = m_reg->view<Cmp::SpawnAreaSprite, Cmp::Position>();
    for ( auto [spawn_entity, spawnarea_cmp, spawn_pos_cmp] : spawnarea_view.each() )
    {
      if ( spawn_pos_cmp.findIntersection( pos_cmp ) )
      {
        // Don't create footstep in spawn area
        return;
      }
    }
    // not realistic to have footsteps inside obstacles unless we have debug collision off
    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
    {
      if ( obstacle_cmp.m_enabled && obstacle_pos_cmp.findIntersection( pos_cmp ) )
      {
        // Don't create footstep in obstacle area
        return;
      }
    }
    SPDLOG_DEBUG( "Adding footstep at position ({}, {})", position.x, position.y );
    auto entity = m_reg->create();

    m_reg->emplace<Cmp::Position>( entity, pos_cmp.position, pos_cmp.size );
    m_reg->emplace<Cmp::Direction>( entity, direction );
    m_reg->emplace<Cmp::FootStepTimer>( entity );
    m_reg->emplace<Cmp::FootStepAlpha>( entity );
    // Reset the timer to enforce discrete steps
    update_clock.restart();
  }
}

void FootstepSystem::update()
{

  // add new footstep for player
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, player, pos_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { continue; }
    add_footstep( pos_cmp, dir_cmp );
  }

  auto view = m_reg->view<Cmp::FootStepTimer, Cmp::FootStepAlpha>();
  for ( auto [entity, timer, alpha] : view.each() )
  {
    if ( timer.m_clock.getElapsedTime() > kFootstepFadeDelay )
    {
      alpha.m_alpha -= kFootstepFadeFactor;
      if ( alpha.m_alpha <= 0.f )
      {
        SPDLOG_DEBUG( "Removing footstep entity {}", int( entity ) );
        m_reg->destroy( entity );
      }
    }
  }
}

} // namespace ProceduralMaze::Sys