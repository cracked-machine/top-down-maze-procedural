#include <FootStepAlpha.hpp>
#include <FootStepTimer.hpp>
#include <MultiSprite.hpp>
#include <Systems/FootstepSystem.hpp>

namespace ProceduralMaze::Sys {

void FootstepSystem::add_footstep( const Cmp::Position &pos_cmp, const Cmp::Direction &direction )
{
  if ( update_clock.getElapsedTime() >= kFootstepAddDelay )
  {
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