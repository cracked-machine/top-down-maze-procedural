#include <AnimSystem.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

void AnimSystem::update( sf::Time deltaTime )
{
  auto anim_view = m_reg->view<Cmp::SpriteAnimation>();
  for ( auto [entity, anim_cmp] : anim_view.each() )
  {
    update_frame( anim_cmp, deltaTime );
  }
}

void AnimSystem::on_anim_reset_frame( const Events::AnimResetFrameEvent &event )
{

  auto anim_cmp = m_reg->try_get<Cmp::SpriteAnimation>( event.m_entity );
  if ( !anim_cmp ) return;
  anim_cmp->m_current_frame = 0;
  anim_cmp->m_elapsed_time = sf::Time::Zero;
}

void AnimSystem::on_anim_direction_change( const Events::AnimDirectionChangeEvent &event )
{
  // SPDLOG_INFO("AnimDirectionChangeEvent");

  auto dir_cmp = m_reg->try_get<Cmp::Direction>( event.m_entity );
  auto anim_cmp = m_reg->try_get<Cmp::SpriteAnimation>( event.m_entity );
  if ( !dir_cmp || !anim_cmp ) return;

  // Set base frame based on direction
  if ( dir_cmp->x == 1 )
  {
    anim_cmp->m_base_frame = 6; // Right-facing base frame
  }
  else if ( dir_cmp->x == -1 )
  {
    anim_cmp->m_base_frame = 9; // Left-facing base frame
  }
  else if ( dir_cmp->y == -1 )
  {
    anim_cmp->m_base_frame = 3; // Up-facing base frame
  }
  else if ( dir_cmp->y == 1 )
  {
    anim_cmp->m_base_frame = 0; // Down-facing base frame
  }
}

void AnimSystem::update_frame( Cmp::SpriteAnimation &anim, sf::Time deltaTime )
{
  anim.m_elapsed_time += deltaTime;

  if ( anim.m_elapsed_time >= anim.m_frame_duration )
  {
    // Increment frame. Wrap around to zero at anim.m_frame_count
    anim.m_current_frame = ( anim.m_current_frame + 1 ) % anim.m_frame_count;
    anim.m_elapsed_time = sf::Time::Zero;
  }
}

} // namespace ProceduralMaze::Sys