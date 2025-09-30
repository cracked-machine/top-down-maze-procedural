#include <AnimSystem.hpp>
#include <EnttDistanceMap.hpp>
#include <LerpPosition.hpp>
#include <NPC.hpp>
#include <PlayableCharacter.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

void AnimSystem::update( sf::Time deltaTime )
{

  // only update animation for NPC that are actively pathfinding
  auto pathfinding_npc_view = m_reg->view<Cmp::NPC, Cmp::LerpPosition, Cmp::SpriteAnimation>();
  for ( [[maybe_unused]] auto [entity, npc_cmp, lerp_pos_cmp, anim_cmp] : pathfinding_npc_view.each() )
  {
    if ( lerp_pos_cmp.m_lerp_factor > 0.f ) { update_frame( anim_cmp, deltaTime ); }
  }

  // always update animation for player if they are moving
  auto moving_player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction, Cmp::SpriteAnimation>();
  for ( auto [entity, pc_cmp, dir_cmp, anim_cmp] : moving_player_view.each() )
  {
    if ( dir_cmp != sf::Vector2f( 0.f, 0.f ) ) { update_frame( anim_cmp, deltaTime ); }
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
  auto pos_cmp = m_reg->try_get<Cmp::Position>( event.m_entity );
  auto anim_cmp = m_reg->try_get<Cmp::SpriteAnimation>( event.m_entity );
  if ( !dir_cmp || !anim_cmp || !pos_cmp ) return;

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
  // Special case:
  // if squeezing diagonally between obstacles, face down
  if ( isDiagonalMovementBetweenObstacles( *pos_cmp, *dir_cmp ) )
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