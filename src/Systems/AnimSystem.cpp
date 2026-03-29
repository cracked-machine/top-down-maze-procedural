#include <Persistent/NpcWitchAnimFramerate.hpp>
#include <Ruin/RuinShadowHand.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Components/Altar/AltarSacrifice.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptPassageSpikeTrap.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomLavaPitCellEffect.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/LootContainer.hpp>
#include <Systems/AnimSystem.hpp>

#include <SFML/System/Time.hpp>
#include <spdlog/spdlog.h>

#include <Components/Altar/AltarSegment.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wormhole/WormholeMultiBlock.hpp>
#include <Components/Wormhole/WormholeSingularity.hpp>
#include <Components/ZOrderValue.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>

namespace ProceduralMaze::Sys
{

void AnimSystem::update( sf::Time dt )
{

  auto anim_view = getReg().view<Cmp::SpriteAnimation, Cmp::Position>( entt::exclude<Cmp::NPC> );
  for ( auto [anim_entt, anim_cmp, pos_cmp] : anim_view.each() )
  {
    if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), pos_cmp ) ) continue;
    if ( anim_cmp.m_animation_active == true )
    {
      const auto &ms = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      update_single_sequence( anim_cmp, dt, ms, sf::seconds( anim_cmp.get_framerate() ) );

      // disable oneshot animations at the end of their sequence
      if ( anim_cmp.m_anim_type == Cmp::AnimType::LOOP ) continue;
      if ( anim_cmp.m_current_frame == ms.get_sprites_per_sequence() - 1 )
      {
        SPDLOG_DEBUG( "Deactivating animation: {}", static_cast<int>( anim_entt ) );
        anim_cmp.m_animation_active = false;
        anim_cmp.m_current_frame = anim_cmp.m_base_frame;
      }
    }
  }

  //   // Shadow Hand Animation
  //   auto ruin_shadowhand_anim_view = getReg().view<Cmp::RuinShadowHand, Cmp::SpriteAnimation, Cmp::Position>();
  //   for ( auto [ruin_shadowhand_entt, ruin_shadowhand_cmp, ruin_shadowhand_anim_cmp, ruin_shadowhand_pos_cmp] : ruin_shadowhand_anim_view.each() )
  //   {
  //     if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), ruin_shadowhand_pos_cmp ) ) continue;
  //     if ( ruin_shadowhand_anim_cmp.m_animation_active == true )
  //     {
  //       const auto &altar_sacrifice_sprite_metadata = m_sprite_factory.get_multisprite_by_type( ruin_shadowhand_anim_cmp.m_sprite_type );
  //       auto frame_rate = sf::seconds( 0.2f );

  //       update_single_sequence( ruin_shadowhand_anim_cmp, globalDeltaTime, altar_sacrifice_sprite_metadata, frame_rate );
  //     }
  //   }

  //   // Grave Pot hit Animation
  //   auto loot_container_anim_view = getReg().view<Cmp::LootContainer, Cmp::SpriteAnimation, Cmp::Position>();
  //   for ( auto [loot_con_entt, loot_con_cmp, loot_con_anim_cmp, loot_con_pos_cmp] : loot_container_anim_view.each() )
  //   {
  //     if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), loot_con_pos_cmp ) ) continue;
  //     if ( loot_con_anim_cmp.m_sprite_type == "POT" and loot_con_anim_cmp.m_animation_active == true )
  //     {
  //       const auto &pot_sprite_metadata = m_sprite_factory.get_multisprite_by_type( loot_con_anim_cmp.m_sprite_type );
  //       auto frame_rate = sf::seconds( 0.2f );

  //       update_single_sequence( loot_con_anim_cmp, globalDeltaTime, pot_sprite_metadata, frame_rate );

  //       // one shot animation then deactivate, this is then destroyed by `DiggingSystem::check_player_smash_pot`
  //       if ( loot_con_anim_cmp.m_current_frame == pot_sprite_metadata.get_sprites_per_sequence() - 1 )
  //       {
  //         SPDLOG_DEBUG( "Deactivating pot animation: {}", static_cast<int>( loot_con_entt ) );
  //         loot_con_anim_cmp.m_animation_active = false;
  //         loot_con_anim_cmp.m_current_frame = loot_con_anim_cmp.m_base_frame;
  //       }
  //     }
  //   }

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
        frame_rate = sf::seconds( Sys::PersistSystem::get<Cmp::Persist::NpcSkeleAnimFramerate>( getReg() ).get_value() );
      }
      else if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) )
      {
        frame_rate = sf::seconds( Sys::PersistSystem::get<Cmp::Persist::NpcGhostAnimFramerate>( getReg() ).get_value() );
      }
      else if ( anim_cmp.m_sprite_type.contains( "NPCWITCH" ) )
      {
        frame_rate = sf::seconds( Sys::PersistSystem::get<Cmp::Persist::NpcWitchAnimFramerate>( getReg() ).get_value() );
      }
      const auto &npc_walk_sequence = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      update_single_sequence( anim_cmp, dt, npc_walk_sequence, frame_rate );
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