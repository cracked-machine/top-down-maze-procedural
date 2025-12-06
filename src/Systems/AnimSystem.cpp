#include <Components/GraveSegment.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/WormholeMultiBlock.hpp>
#include <spdlog/spdlog.h>

#include <SFML/System/Time.hpp>

#include <Components/AltarSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/NpcDeathPosition.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WormholeSingularity.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

void AnimSystem::update( sf::Time globalDeltaTime )
{

  // Shrine Animation
  auto shrine_view = getReg().view<Cmp::AltarSegment, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, shrine_cmp, anim_cmp, pos_cmp] : shrine_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      const auto &shrine_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, globalDeltaTime, shrine_sprite_metadata, frame_rate );
    }
  }

  // Grave Animation
  auto grave_view = getReg().view<Cmp::GraveSegment, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, grave_cmp, anim_cmp, pos_cmp] : grave_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      SPDLOG_DEBUG( "Updating Grave animation for entity {}", static_cast<int>( entity ) );
      const auto &grave_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, globalDeltaTime, grave_sprite_metadata, frame_rate );
    }
  }

  // NPC Movement: only update animation for NPC that are actively pathfinding
  auto pathfinding_npc_view = getReg().view<Cmp::NPC, Cmp::LerpPosition, Cmp::SpriteAnimation, Cmp::Position>();
  for ( [[maybe_unused]] auto [entity, npc_cmp, lerp_pos_cmp, anim_cmp, pos_cmp] : pathfinding_npc_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( lerp_pos_cmp.m_lerp_factor > 0.f )
    {

      sf::Time frame_rate = sf::Time::Zero;
      if ( anim_cmp.m_sprite_type.contains( "NPCSKELE" ) )
      {
        frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::NpcSkeleAnimFramerate>().get_value() );
      }
      else if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) )
      {
        frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::NpcGhostAnimFramerate>().get_value() );
      }
      const auto &npc_walk_sequence = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      update_single_sequence( anim_cmp, globalDeltaTime, npc_walk_sequence, frame_rate );
    }
  }

  // Player Movement
  // TODO: Add death animations depending on the mortality state
  auto moving_player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Direction, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, pc_cmp, dir_cmp, anim_cmp, pos_cmp] : moving_player_view.each() )
  {

    if ( not anim_cmp.m_animation_active ) continue;
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    auto frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::PlayerAnimFramerate>().get_value() );
    const auto &player_walk_sequence = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
    update_single_sequence( anim_cmp, globalDeltaTime, player_walk_sequence, frame_rate );
  }

  // Wormhole
  const auto wormhole_view = getReg().view<Cmp::WormholeMultiBlock, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, wormhole_cmp, anim_cmp, pos_cmp] : wormhole_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;

    const auto &wormhole_sprite_metadata = m_sprite_factory.get_multisprite_by_type( "WORMHOLE" );
    auto frame_rate = sf::seconds( get_persistent_component<Cmp::Persistent::WormholeAnimFramerate>().get_value() );

    update_single_sequence( anim_cmp, globalDeltaTime, wormhole_sprite_metadata, frame_rate );
  }

  // NPC Death Explosion Animation
  auto explosion_view = getReg().view<Cmp::NpcDeathPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, explosion_cmp, anim_cmp] : explosion_view.each() )
  {
    const auto &explosion_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
    auto frame_rate = sf::seconds( getReg().ctx().get<Cmp::Persistent::NpcDeathAnimFramerate>().get_value() );

    SPDLOG_DEBUG( "Explosion animation active for entity {} - current_frame: {}, sprites_per_frame: {}, "
                  "sprites_per_sequence: {}, frame_rate: {}s",
                  static_cast<int>( entity ), anim_cmp.m_current_frame, explosion_sprite_metadata.get_sprites_per_frame(),
                  explosion_sprite_metadata.get_sprites_per_sequence(), frame_rate.asSeconds() );

    // Update the frame first
    update_single_sequence( anim_cmp, globalDeltaTime, explosion_sprite_metadata, frame_rate );

    SPDLOG_DEBUG( "After update_frame - current_frame: {}, elapsed_time: {}s", anim_cmp.m_current_frame, anim_cmp.m_elapsed_time.asSeconds() );

    // have we completed the animation?
    if ( anim_cmp.m_current_frame == explosion_sprite_metadata.get_sprites_per_sequence() - 1 )
    {
      getReg().remove<Cmp::NpcDeathPosition>( entity );
      getReg().remove<Cmp::SpriteAnimation>( entity );
      getReg().remove<Cmp::ZOrderValue>( entity );
      getReg().remove<Cmp::Position>( entity );
      SPDLOG_DEBUG( "Explosion animation complete, removing component from entity {}", static_cast<int>( entity ) );
      continue;
    }
  }
}

void AnimSystem::update_single_sequence( Cmp::SpriteAnimation &anim, sf::Time globalDeltaTime, const Sprites::MultiSprite &ms, sf::Time frame_rate )
{
  anim.m_elapsed_time += globalDeltaTime;

  if ( anim.m_elapsed_time >= frame_rate )
  {
    // SPDLOG_INFO( "Before: current_frame={}, elapsed_time={}s, sprites_per_frame={}, total_sprites={}", anim.m_current_frame,
    //              anim.m_elapsed_time.asSeconds(), sprites_per_frame, total_sprites );

    unsigned int num_animation_frames = ms.get_sprites_per_sequence() / ms.get_sprites_per_frame();
    unsigned int current_anim_frame = anim.m_current_frame / ms.get_sprites_per_frame();
    unsigned int next_anim_frame = ( current_anim_frame + 1 ) % num_animation_frames;

    if ( next_anim_frame == 0 ) { next_anim_frame = anim.m_base_frame; }

    anim.m_current_frame = next_anim_frame * ms.get_sprites_per_frame();
    anim.m_elapsed_time -= frame_rate;

    // SPDLOG_INFO( "After: current_frame={}, base_frame={}", anim.m_current_frame, anim.m_base_frame );
  }
}

void AnimSystem::update_grouped_sequences( Cmp::SpriteAnimation &anim, sf::Time globalDeltaTime, const Sprites::MultiSprite &ms, sf::Time frame_rate )
{
  anim.m_elapsed_time += globalDeltaTime;

  if ( anim.m_elapsed_time >= frame_rate )
  {
    // Increment frame. Wrap around to zero at anim.m_frame_count
    anim.m_current_frame = ( anim.m_current_frame + ms.get_sprites_per_frame() ) % ms.get_sprites_per_sequence();

    // Subtract frame_rate instead of resetting to Zero to maintain precise timing
    // i.e. this carries the time overflow from previous update:
    // Example: if frame_rate = 0.1 seconds (100ms per frame)
    // Frame N: deltaTime = 0.05s
    //   elapsed_time = 0.07 + 0.05 = 0.12s  // >= 0.1, advance frame!
    //   elapsed_time = 0.12 - 0.10 = 0.02s  // Keep the 0.02s "overflow"
    anim.m_elapsed_time -= frame_rate;
  }
}

} // namespace ProceduralMaze::Sys