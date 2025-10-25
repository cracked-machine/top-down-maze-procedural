#include <AnimSystem.hpp>
#include <EnttDistanceMap.hpp>
#include <LerpPosition.hpp>
#include <NPC.hpp>
#include <NpcDeathPosition.hpp>
#include <Persistent/NpcAnimFramerate.hpp>
#include <Persistent/NpcDeathAnimFramerate.hpp>
#include <Persistent/PlayerAnimFramerate.hpp>
#include <Persistent/WormholeAnimFramerate.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Time.hpp>
#include <SpriteAnimation.hpp>
#include <SpriteFactory.hpp>
#include <Systems/RenderSystem.hpp>
#include <Wormhole.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

void AnimSystem::update( sf::Time deltaTime )
{
  using namespace Sprites;
  auto &factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();

  // ReservedPosition Animation
  auto reserved_view = m_reg->view<Cmp::ReservedPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, reserved_cmp, anim_cmp] : reserved_view.each() )
  {
    if ( !is_visible_in_view( RenderSystem::getWindow().getView(), reserved_cmp ) ) continue;
    if ( reserved_cmp.is_animated() )
    {
      SPDLOG_DEBUG( "Updating ReservedPosition animation for entity {}", static_cast<int>( entity ) );
      auto reserved_sprite_metadata = factory->get_multisprite_by_type( reserved_cmp.m_type );
      auto sprites_per_frame = reserved_sprite_metadata->get_sprites_per_frame();
      auto sprites_per_sequence = reserved_sprite_metadata->get_sprites_per_sequence();
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, deltaTime, sprites_per_frame, sprites_per_sequence, frame_rate );
    }
  }

  // NPC Movement: only update animation for NPC that are actively pathfinding
  auto pathfinding_npc_view = m_reg->view<Cmp::NPC, Cmp::LerpPosition, Cmp::SpriteAnimation, Cmp::Position>();
  for ( [[maybe_unused]] auto [entity, npc_cmp, lerp_pos_cmp, anim_cmp, pos_cmp] : pathfinding_npc_view.each() )
  {
    if ( !is_visible_in_view( RenderSystem::getWindow().getView(), pos_cmp ) ) continue;
    if ( lerp_pos_cmp.m_lerp_factor > 0.f )
    {
      auto &factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
      auto npc_sprite_metadata = factory->get_multisprite_by_type( "NPC" );
      auto sprites_per_frame = npc_sprite_metadata->get_sprites_per_frame();
      auto sprites_per_sequence = npc_sprite_metadata->get_sprites_per_sequence();
      auto frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::NpcAnimFramerate>().get_value() );

      update_single_sequence( anim_cmp, deltaTime, sprites_per_frame, sprites_per_sequence, frame_rate );
    }
  }

  // Player Movement:always update animation for player if they are moving
  auto moving_player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, pc_cmp, dir_cmp, anim_cmp, pos_cmp] : moving_player_view.each() )
  {
    if ( !is_visible_in_view( RenderSystem::getWindow().getView(), pos_cmp ) ) continue;
    if ( dir_cmp != sf::Vector2f( 0.f, 0.f ) )
    {
      auto &factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
      auto player_sprite_metadata = factory->get_multisprite_by_type( "PLAYER" );
      auto sprites_per_frame = player_sprite_metadata->get_sprites_per_frame();
      auto sprites_per_sequence = player_sprite_metadata->get_sprites_per_sequence();
      auto frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::PlayerAnimFramerate>().get_value() );

      update_grouped_sequences( anim_cmp, deltaTime, sprites_per_frame, sprites_per_sequence, frame_rate );
    }
  }

  // Wormhole
  auto wormhole_view = m_reg->view<Cmp::Wormhole, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, wormhole_cmp, anim_cmp, pos_cmp] : wormhole_view.each() )
  {
    if ( !is_visible_in_view( RenderSystem::getWindow().getView(), pos_cmp ) ) continue;

    auto wormhole_sprite_metadata = factory->get_multisprite_by_type( "WORMHOLE" );
    auto sprites_per_frame = wormhole_sprite_metadata->get_sprites_per_frame();
    auto sprites_per_sequence = wormhole_sprite_metadata->get_sprites_per_sequence();
    auto frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::WormholeAnimFramerate>().get_value() );

    update_single_sequence( anim_cmp, deltaTime, sprites_per_frame, sprites_per_sequence, frame_rate );
  }

  // NPC Death Explosion Animation
  auto explosion_view = m_reg->view<Cmp::NpcDeathPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, explosion_cmp, anim_cmp] : explosion_view.each() )
  {
    auto explosion_sprite_metadata = factory->get_multisprite_by_type( "EXPLOSION" );
    auto sprites_per_frame = explosion_sprite_metadata->get_sprites_per_frame();
    auto sprites_per_sequence = explosion_sprite_metadata->get_sprites_per_sequence();
    auto frame_rate = sf::seconds( m_reg->ctx().get<Cmp::Persistent::NpcDeathAnimFramerate>().get_value() );

    SPDLOG_DEBUG( "Explosion animation active for entity {} - current_frame: {}, sprites_per_frame: {}, "
                  "sprites_per_sequence: {}, frame_rate: {}s",
                  static_cast<int>( entity ), anim_cmp.m_current_frame, sprites_per_frame, sprites_per_sequence,
                  frame_rate.asSeconds() );

    // Update the frame first
    update_single_sequence( anim_cmp, deltaTime, sprites_per_frame, sprites_per_sequence, frame_rate );

    SPDLOG_DEBUG( "After update_frame - current_frame: {}, elapsed_time: {}s", anim_cmp.m_current_frame,
                  anim_cmp.m_elapsed_time.asSeconds() );

    // have we completed the animation?
    if ( anim_cmp.m_current_frame == sprites_per_sequence - 1 )
    {
      m_reg->remove<Cmp::NpcDeathPosition>( entity );
      m_reg->remove<Cmp::SpriteAnimation>( entity );
      SPDLOG_DEBUG( "Explosion animation complete, removing component from entity {}", static_cast<int>( entity ) );
      continue;
    }
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

void AnimSystem::update_grouped_sequences( Cmp::SpriteAnimation &anim, sf::Time deltaTime,
                                           const unsigned int sprites_per_frame, const unsigned int frames_per_group,
                                           sf::Time frame_rate )
{
  anim.m_elapsed_time += deltaTime;

  if ( anim.m_elapsed_time >= frame_rate )
  {

    // Increment frame. Wrap around to zero at anim.m_frame_count
    anim.m_current_frame = ( anim.m_current_frame + sprites_per_frame ) % frames_per_group;

    // Subtract frame_rate instead of resetting to Zero to maintain precise timing
    // i.e. this carries the time overflow from previous update:
    // Example: if frame_rate = 0.1 seconds (100ms per frame)
    // Frame N: deltaTime = 0.05s
    //   elapsed_time = 0.07 + 0.05 = 0.12s  // >= 0.1, advance frame!
    //   elapsed_time = 0.12 - 0.10 = 0.02s  // Keep the 0.02s "overflow"
    anim.m_elapsed_time -= frame_rate;
  }
}

void AnimSystem::update_single_sequence( Cmp::SpriteAnimation &anim, sf::Time deltaTime,
                                         const unsigned int sprites_per_frame, const unsigned int total_sprites,
                                         sf::Time frame_rate )
{
  anim.m_elapsed_time += deltaTime;

  if ( anim.m_elapsed_time >= frame_rate )
  {
    // For single sequence animations with base frame offset
    // Calculate how many animation frames exist
    unsigned int num_animation_frames = total_sprites / sprites_per_frame; // 20/4 = 5 frames

    // Get current animation frame index (0,1,2,3,4)
    unsigned int current_anim_frame = anim.m_current_frame / sprites_per_frame;

    // Advance to next animation frame, wrapping around
    unsigned int next_anim_frame = ( current_anim_frame + 1 ) % num_animation_frames;

    // If we wrapped around, start from base frame, otherwise continue sequence
    if ( next_anim_frame == 0 )
    {
      next_anim_frame = anim.m_base_frame; // Start from base frame after wrap
    }

    // Convert back to sprite index
    anim.m_current_frame = next_anim_frame * sprites_per_frame;

    anim.m_elapsed_time -= frame_rate;
  }
}

// void AnimSystem::update_frame( Cmp::SpriteAnimation &anim, sf::Time deltaTime, const unsigned int sprites_per_frame,
//                                const unsigned int sprites_per_sequence, sf::Time frame_rate )
// {
//   anim.m_elapsed_time += deltaTime;

//   if ( anim.m_elapsed_time >= frame_rate )
//   {
//     // Increment frame. Wrap around to zero at anim.m_frame_count
//     anim.m_current_frame = ( anim.m_current_frame + sprites_per_frame ) % sprites_per_sequence;

//     // Subtract frame_rate instead of resetting to Zero to maintain precise timing
//     // i.e. this carries the time overflow from previous update:
//     // Example: if frame_rate = 0.1 seconds (100ms per frame)
//     // Frame N: deltaTime = 0.05s
//     //   elapsed_time = 0.07 + 0.05 = 0.12s  // >= 0.1, advance frame!
//     //   elapsed_time = 0.12 - 0.10 = 0.02s  // Keep the 0.02s "overflow"
//     anim.m_elapsed_time -= frame_rate;
//   }
// }

} // namespace ProceduralMaze::Sys