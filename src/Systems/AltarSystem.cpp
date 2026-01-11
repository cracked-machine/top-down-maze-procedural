#include <Components/Inventory/CarryItem.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Systems/AltarSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPC.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/PersistSystem.hpp>
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
      Cmp::Position new_pos( altar_cmp.getCenter() - sf::Vector2{ 8.f, 8.f }, Constants::kGridSquareSizePixelsF );
      Factory::createCarryItem( getReg(), new_pos, inventory_type, 2.f );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
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

    if ( altar_cmp.are_powers_active() )
    {
      SPDLOG_DEBUG( "Checking for special power activation." );
      auto anim_sprite_cmp = getReg().try_get<Cmp::SpriteAnimation>( altar_entity );
      if ( anim_sprite_cmp && m_altar_activation_clock.getElapsedTime() > m_altar_activation_cooldown ) { activate_altar_special_power(); }
    }
  }
}

bool AltarSystem::activate_altar_special_power()
{

  for ( auto [pc_entity, pc_relic_count_cmp] : getReg().view<Cmp::PlayerRelicCount>().each() )
  {

    if ( pc_relic_count_cmp.get_count() < 1 ) return false;

    SPDLOG_DEBUG( "Activating altar special power!" );
    // consume a relic
    pc_relic_count_cmp.decrement_count( 1 );

    SPDLOG_DEBUG( "Special Power: Re-enable all nearby obstacles!" );
    auto obstacle_view = getReg().view<Cmp::DestroyedObstacle, Cmp::Position>(
        entt::exclude<Cmp::PlayableCharacter, Cmp::NPC, Cmp::LootContainer, Cmp::Loot, Cmp::ReservedPosition> );

    for ( auto [destroyed_entity, destroyed_cmp, destroyed_pos_cmp] : obstacle_view.each() )
    {
      if ( Utils::is_visible_in_view( RenderSystem::getGameView(), destroyed_pos_cmp ) )
      {
        auto obst_cmp = getReg().try_get<Cmp::Obstacle>( destroyed_entity );
        if ( not obst_cmp )
        {
          // skip if player or npc is occupying the position
          bool skip_position = false;
          auto npc_view = getReg().view<Cmp::NPC, Cmp::Position>();
          for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
          {
            if ( npc_pos_cmp.findIntersection( destroyed_pos_cmp ) ) skip_position = true;
          }
          auto playable_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
          for ( auto [playable_entity, playable_cmp, playable_pos_cmp] : playable_view.each() )
          {
            if ( playable_pos_cmp.findIntersection( destroyed_pos_cmp ) ) skip_position = true;
          }
          if ( skip_position ) continue;

          // clang-format off
          auto [obst_type, rand_obst_tex_idx] = 
            m_sprite_factory.get_random_type_and_texture_index( { 
              "ROCK"
            } );
          // clang-format on

          Factory::createObstacle( getReg(), destroyed_entity, destroyed_pos_cmp, obst_type, rand_obst_tex_idx, destroyed_pos_cmp.position.y );
        }
      }
    }
  }

  SPDLOG_DEBUG( "Special Power: Kill all nearby NPCs!" );
  auto npc_view = getReg().view<Cmp::NPC, Cmp::Position>();
  for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
  {
    if ( Utils::is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Killed NPC at ({}, {})", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
      auto loot_entity = Factory::destroyNPC( getReg(), npc_entity );
      if ( loot_entity != entt::null )
      {
        SPDLOG_INFO( "Dropped RELIC_DROP loot at NPC death position." );
        m_sound_bank.get_effect( "drop_relic" ).play();
      }
    }
  }

  SPDLOG_DEBUG( "Special Power: Open all loot containers!" );
  auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position>();
  for ( auto [lc_entity, lc_cmp, lc_pos_cmp] : loot_container_view.each() )
  {
    if ( Utils::is_visible_in_view( RenderSystem::getGameView(), lc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Opened loot container at ({}, {})", lc_pos_cmp.position.x, lc_pos_cmp.position.y );
      auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );

      // clang-format off
      auto loot_entt = Factory::createLootDrop( 
        getReg(), 
        Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),                                        
        sf::FloatRect{ lc_pos_cmp.position, lc_pos_cmp.size }, 
        Factory::IncludePack<>{},
        Factory::ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition>{} );
      // clang-format on

      if ( loot_entt != entt::null ) { m_sound_bank.get_effect( "break_pot" ).play(); }

      Factory::destroyLootContainer( getReg(), lc_entity );
    }
  }

  m_altar_activation_clock.restart();
  return true;
}

} // namespace ProceduralMaze::Sys
