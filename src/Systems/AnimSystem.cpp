#include <Components/AltarSacrifice.hpp>
#include <Components/CryptChest.hpp>
#include <Components/CryptPassageSpikeTrap.hpp>
#include <Components/CryptRoomLavaPitCell.hpp>
#include <Components/CryptRoomLavaPitCellEffect.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/LootContainer.hpp>
#include <Systems/AnimSystem.hpp>

#include <SFML/System/Time.hpp>
#include <spdlog/spdlog.h>

#include <Components/AltarSegment.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WormholeMultiBlock.hpp>
#include <Components/WormholeSingularity.hpp>
#include <Components/ZOrderValue.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>

namespace ProceduralMaze::Sys
{

void AnimSystem::update( sf::Time globalDeltaTime )
{

  // Grave Pot hit Animation
  auto altar_sacrifice_anim_view = getReg().view<Cmp::AltarSacrifice, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [altar_sacrifice_entt, altar_sacrifice_cmp, altar_sacrifice_anim_cmp, altar_sacrifice_pos_cmp] : altar_sacrifice_anim_view.each() )
  {
    if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), altar_sacrifice_pos_cmp ) ) continue;
    if ( altar_sacrifice_anim_cmp.m_sprite_type == "ALTAR.sacrifice.anim" and altar_sacrifice_anim_cmp.m_animation_active == true )
    {
      const auto &altar_sacrifice_sprite_metadata = m_sprite_factory.get_multisprite_by_type( altar_sacrifice_anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.2f );

      update_single_sequence( altar_sacrifice_anim_cmp, globalDeltaTime, altar_sacrifice_sprite_metadata, frame_rate );

      // one shot animation then deactivate, this is then destroyed by `AltarSystem`
      if ( altar_sacrifice_anim_cmp.m_current_frame == altar_sacrifice_sprite_metadata.get_sprites_per_sequence() - 1 )
      {
        SPDLOG_DEBUG( "Deactivating altar sacrifice animation: {}", static_cast<int>( loot_con_entt ) );
        altar_sacrifice_anim_cmp.m_animation_active = false;
        altar_sacrifice_anim_cmp.m_current_frame = altar_sacrifice_anim_cmp.m_base_frame;
      }
    }
  }

  // Grave Pot hit Animation
  auto loot_container_anim_view = getReg().view<Cmp::LootContainer, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [loot_con_entt, loot_con_cmp, loot_con_anim_cmp, loot_con_pos_cmp] : loot_container_anim_view.each() )
  {
    if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), loot_con_pos_cmp ) ) continue;
    if ( loot_con_anim_cmp.m_sprite_type == "POT" and loot_con_anim_cmp.m_animation_active == true )
    {
      const auto &pot_sprite_metadata = m_sprite_factory.get_multisprite_by_type( loot_con_anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.2f );

      update_single_sequence( loot_con_anim_cmp, globalDeltaTime, pot_sprite_metadata, frame_rate );

      // one shot animation then deactivate, this is then destroyed by `DiggingSystem::check_player_smash_pot`
      if ( loot_con_anim_cmp.m_current_frame == pot_sprite_metadata.get_sprites_per_sequence() - 1 )
      {
        SPDLOG_DEBUG( "Deactivating pot animation: {}", static_cast<int>( loot_con_entt ) );
        loot_con_anim_cmp.m_animation_active = false;
        loot_con_anim_cmp.m_current_frame = loot_con_anim_cmp.m_base_frame;
      }
    }
  }

  // Crypt Spike Trap Animation
  auto spiketrap_view = getReg().view<Cmp::CryptPassageSpikeTrap, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [spike_trap_entt, spike_trap_cmp, anim_cmp, pos_cmp] : spiketrap_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      const auto &spiketrap_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.2f );

      update_single_sequence( anim_cmp, globalDeltaTime, spiketrap_sprite_metadata, frame_rate );

      // one shot animation, then pause for N seconds,
      // then wait for player proximity to reactivate. See CryptSystem::checkSpikeTrapActivationByProximity().
      if ( anim_cmp.m_current_frame == spiketrap_sprite_metadata.get_sprites_per_sequence() - 1 )
      {
        SPDLOG_DEBUG( "Deactivating spike: {}", static_cast<int>( spike_trap_entt ) );
        anim_cmp.m_animation_active = false;
        spike_trap_cmp.m_cooldown_timer.restart();
        anim_cmp.m_current_frame = anim_cmp.m_base_frame;
      }
    }
  }

  // Crypt chest Animation
  auto crypt_chest_view = getReg().view<Cmp::CryptChest, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [crypt_chest_entt, crypt_chest_cmp, anim_cmp, pos_cmp] : crypt_chest_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      const auto &spiketrap_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, globalDeltaTime, spiketrap_sprite_metadata, frame_rate );

      // one shot animation, then disable
      // See CryptSystem::check_chest_activation()
      if ( anim_cmp.m_current_frame == spiketrap_sprite_metadata.get_sprites_per_sequence() - 1 ) { anim_cmp.m_animation_active = false; }
    }
  }

  // Crypt lava cell animation effect
  auto crypt_lava_view = getReg().view<Cmp::CryptRoomLavaPitCellEffect, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [crypt_lava_anim_entt, crypt_lava_anim_cmp, anim_cmp, pos_cmp] : crypt_lava_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      const auto &spiketrap_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, globalDeltaTime, spiketrap_sprite_metadata, frame_rate );

      // one shot animation, then disable. See CryptSystem::doLavaPitAnimation()
      if ( anim_cmp.m_current_frame == spiketrap_sprite_metadata.get_sprites_per_sequence() - 1 ) { anim_cmp.m_animation_active = false; }
    }
  }

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

  // HolyWell Animation
  auto holywell_view = getReg().view<Cmp::HolyWellMultiBlock, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, holywell_cmp, anim_cmp, pos_cmp] : holywell_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active )
    {
      const auto &holywell_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      auto frame_rate = sf::seconds( 0.1f );

      update_single_sequence( anim_cmp, globalDeltaTime, holywell_sprite_metadata, frame_rate );
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
        frame_rate = sf::seconds( Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcSkeleAnimFramerate>( getReg() ).get_value() );
      }
      else if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) )
      {
        frame_rate = sf::seconds( Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcGhostAnimFramerate>( getReg() ).get_value() );
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
    auto frame_rate = sf::seconds( Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerAnimFramerate>( getReg() ).get_value() );
    const auto &player_walk_sequence = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
    update_single_sequence( anim_cmp, globalDeltaTime, player_walk_sequence, frame_rate );
  }

  // Wormhole
  const auto wormhole_view = getReg().view<Cmp::WormholeMultiBlock, Cmp::SpriteAnimation, Cmp::Position>();
  for ( auto [entity, wormhole_cmp, anim_cmp, pos_cmp] : wormhole_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;

    const auto &wormhole_sprite_metadata = m_sprite_factory.get_multisprite_by_type( "WORMHOLE" );
    auto frame_rate = sf::seconds( Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WormholeAnimFramerate>( getReg() ).get_value() );

    update_single_sequence( anim_cmp, globalDeltaTime, wormhole_sprite_metadata, frame_rate );
  }

  // NPC Death Explosion Animation
  auto explosion_view = getReg().view<Cmp::DeathPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, explosion_cmp, anim_cmp] : explosion_view.each() )
  {
    const auto &explosion_sprite_metadata = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
    auto frame_rate = sf::seconds( getReg().ctx().get<Cmp::Persist::NpcDeathAnimFramerate>().get_value() );

    SPDLOG_DEBUG( "Explosion animation active for entity {} - current_frame: {}, sprites_per_frame: {}, "
                  "sprites_per_sequence: {}, frame_rate: {}s",
                  static_cast<int>( entity ), anim_cmp.m_current_frame, explosion_sprite_metadata.get_sprites_per_frame(),
                  explosion_sprite_metadata.get_sprites_per_sequence(), frame_rate.asSeconds() );

    // Update the frame first
    update_single_sequence( anim_cmp, globalDeltaTime, explosion_sprite_metadata, frame_rate, AnimType::ONESHOT );

    SPDLOG_DEBUG( "After update_frame - current_frame: {}, elapsed_time: {}s", anim_cmp.m_current_frame, anim_cmp.m_elapsed_time.asSeconds() );

    // have we completed the animation?
    if ( anim_cmp.m_current_frame == explosion_sprite_metadata.get_sprites_per_sequence() - 1 )
    {
      getReg().remove<Cmp::DeathPosition>( entity );
      getReg().remove<Cmp::SpriteAnimation>( entity );
      getReg().remove<Cmp::ZOrderValue>( entity );
      getReg().remove<Cmp::Position>( entity );
      SPDLOG_DEBUG( "Explosion animation complete, removing component from entity {}", static_cast<int>( entity ) );
      continue;
    }
  }
}

void AnimSystem::update_single_sequence( Cmp::SpriteAnimation &anim, sf::Time globalDeltaTime, const Sprites::MultiSprite &ms, sf::Time frame_rate,
                                         AnimType type )
{
  anim.m_elapsed_time += globalDeltaTime;

  if ( anim.m_elapsed_time >= frame_rate )
  {
    // SPDLOG_INFO( "Before: current_frame={}, elapsed_time={}s, sprites_per_frame={}, total_sprites = {} ", anim.m_current_frame,
    //              anim.m_elapsed_time.asSeconds(), ms.get_sprites_per_frame(), ms.get_sprites_per_sequence() );

    unsigned int num_animation_frames = ms.get_sprites_per_sequence() / ms.get_sprites_per_frame();
    unsigned int current_anim_frame = anim.m_current_frame / ms.get_sprites_per_frame();
    unsigned int next_anim_frame = ( current_anim_frame + 1 ) % num_animation_frames;
    // SPDLOG_INFO( "Next: next_anim_frame={}", next_anim_frame );

    // Only reset to base frame for looping animations, not for one-shot animations like explosions
    if ( next_anim_frame == 0 && type == AnimSystem::AnimType::ONESHOT ) { next_anim_frame = anim.m_base_frame; }

    anim.m_current_frame = next_anim_frame * ms.get_sprites_per_frame();
    anim.m_elapsed_time -= frame_rate;

    // SPDLOG_INFO( "After: current_frame={}, base_frame={}", anim.m_current_frame, anim.m_base_frame );
  }
}

} // namespace ProceduralMaze::Sys