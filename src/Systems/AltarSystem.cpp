#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Components/Altar/AltarSacrifice.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Factory/AltarFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Systems/AltarSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

AltarSystem::AltarSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void AltarSystem::check_player_collision()
{
  // tidy up any dead altar sacrifice animations
  auto altar_sacrifice_view = getReg().view<Cmp::AltarSacrifice, Cmp::SpriteAnimation>();
  for ( auto [altar_sacrifice_entt, altar_sacrifice_cmp, altar_sacrifice_anim_cmp] : altar_sacrifice_view.each() )
  {
    if ( not altar_sacrifice_anim_cmp.m_animation_active )
    {
      if ( getReg().valid( altar_sacrifice_entt ) ) { getReg().destroy( altar_sacrifice_entt ); }
    }
  }

  auto altar_view = getReg().view<Cmp::AltarMultiBlock>();
  auto player_hitbox = Cmp::RectBounds( Utils::get_player_position( getReg() ).position, Constants::kGridSquareSizePixelsF, 1.5f );

  for ( auto [altar_entity, altar_cmp] : altar_view.each() )
  {
    if ( player_hitbox.findIntersection( altar_cmp ) )
    {
      SPDLOG_DEBUG( "Player collided with Altar at ({}, {})", altar_cmp.position.x, altar_cmp.position.y );
      check_player_altar_activation( altar_entity, altar_cmp );
    }
  }
}

void AltarSystem::check_player_altar_activation( entt::entity altar_entity, Cmp::AltarMultiBlock &altar_cmp )
{

  SPDLOG_DEBUG( "Checking altar activation: {}/{}", altar_cmp.get_activation_count(), altar_cmp.get_activation_threshold() );
  if ( altar_cmp.get_activation_count() < altar_cmp.get_activation_threshold() )
  {
    auto [inventory_entt, inventory_type] = Utils::get_player_inventory_type( getReg() );
    if ( not inventory_type.contains( "CARRYITEM.relic" ) ) return;

    auto common_activation = [&]()
    {
      Factory::destroyInventory( getReg(), inventory_type );
      float altar_sacrifice_anim_height = m_sprite_factory.get_multisprite_by_type( "ALTAR.sacrifice.anim" ).getSpriteSizePixels().y;
      // get the center (topleft coord), then adjust to center the altar_sacrifice_anim, then adjust for altar_sacrifice_anim height
      Cmp::Position new_pos( altar_cmp.getCenter() - sf::Vector2{ 8.f, 8.f } - sf::Vector2{ 0.f, altar_sacrifice_anim_height },
                             Constants::kGridSquareSizePixelsF );
      // Factory::createCarryItem( getReg(), new_pos, inventory_type, 2.f );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      Factory::createAltarSacrificeAnimation( getReg(), new_pos );
    };

    switch ( altar_cmp.get_activation_count() )
    {
      case 0:
        getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_activation_count( 1 ); } );
        getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "ALTAR.one"; } );
        SPDLOG_DEBUG( "Altar activated to state ONE." );
        common_activation();
        break;
      case 1:
        getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_activation_count( 2 ); } );
        getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "ALTAR.two"; } );
        SPDLOG_DEBUG( "Altar activated to state TWO." );
        common_activation();
        break;

      case 2:
        getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_activation_count( 3 ); } );
        getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "ALTAR.three"; } );
        SPDLOG_DEBUG( "Altar activated to state THREE." );
        common_activation();
        break;
      case 3:
        getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_activation_count( 4 ); } );
        getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "ALTAR.four"; } );
        SPDLOG_DEBUG( "Altar activated to state FOUR." );
        common_activation();
        break;
      default:
        break;
    }
  }

  // allow this condition to be met in the same pass as the candle activation above
  if ( altar_cmp.get_activation_count() >= altar_cmp.get_activation_threshold() )
  {
    if ( not altar_cmp.are_powers_active() )
    {
      SPDLOG_DEBUG( "Altar fully activated!" );

      // drop an exit or crypt key
      Cmp::RandomInt key_picker( 0, 1 );
      auto key_choice = key_picker.gen();
      entt::entity key_entt = entt::null;

      // dont keep spawning exit keys if the exit is was already open
      if ( not Utils::is_graveyard_exit_locked( getReg() ) )
      {
        key_entt = Factory::createCarryItem( getReg(), Utils::get_player_position( getReg() ), "CARRYITEM.cryptkey" );
      }
      else
      {
        // otherwise if the exit is locked, its 50/50
        if ( key_choice == 0 ) { key_entt = Factory::createCarryItem( getReg(), Utils::get_player_position( getReg() ), "CARRYITEM.exitkey" ); }
        else { key_entt = Factory::createCarryItem( getReg(), Utils::get_player_position( getReg() ), "CARRYITEM.cryptkey" ); }
      }

      if ( key_entt != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }

      altar_cmp.set_powers_active();
      m_altar_activation_clock.restart();
    }
  }
}

} // namespace ProceduralMaze::Sys
