#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSacrifice.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Inventory/FlashUIInventory.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Particle/Flame.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Stats/SacrificeAction.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/AltarFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <stdexcept>

namespace Game::Sys
{

AltarSystem::AltarSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&AltarSystem::on_player_action>( this );
}

void AltarSystem::on_player_action( Events::PlayerActionEvent ev )
{
  if ( ev.action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto altar_view = reg().view<Cmp::AltarMultiBlock>();
  auto player_hitbox = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ).position, Constants::kGridSizePxF, 1.5f );

  for ( auto [altar_entity, altar_cmp] : altar_view.each() )
  {
    if ( not player_hitbox.findIntersection( altar_cmp ) ) continue;

    SPDLOG_DEBUG( "Player collided with Altar at ({}, {})", altar_cmp.position.x, altar_cmp.position.y );
    check_player_altar_activation( altar_entity, altar_cmp );
  }
}

void AltarSystem::check_player_collision()
{
  // tidy up any dead altar sacrifice animations
  auto altar_sacrifice_view = reg().view<Cmp::AltarSacrifice, Cmp::AnimData>();
  for ( auto [altar_sacrifice_entt, altar_sacrifice_cmp, altar_sacrifice_anim_cmp] : altar_sacrifice_view.each() )
  {
    if ( not altar_sacrifice_anim_cmp.m_enabled )
    {
      if ( reg().valid( altar_sacrifice_entt ) ) { reg().destroy( altar_sacrifice_entt ); }
    }
  }
}

void AltarSystem::check_player_altar_activation( entt::entity altar_entity, Cmp::AltarMultiBlock &altar_cmp )
{
  if ( m_altar_activation_clock.getElapsedTime() < kActivationCooldownSeconds ) return;

  auto *altar_uuid_cmp = reg().try_get<Cmp::UUID>( altar_entity );
  if ( not altar_uuid_cmp ) throw std::runtime_error( "Altar does not have UUID" );

  auto [sacrifice_entt, sacrifice_type] = Utils::Player::get_inventory_type( reg() );

  enum class SacrificeAnimType { RELIC, KEY };
  auto common_activation = [&]( SacrificeAnimType sacrifice_anim_type )
  {
    Sprites::SpriteMetaType sprite_type;
    if ( sacrifice_anim_type == SacrificeAnimType::RELIC ) { sprite_type = "sprite.graveyard.altar.relic.anim"; }
    else if ( sacrifice_anim_type == SacrificeAnimType::KEY ) { sprite_type = "sprite.graveyard.altar.key.anim"; }

    Factory::destroy_inventory( reg(), sacrifice_type );

    float altar_sacrifice_anim_height = m_sprite_factory.get_spritesheet_by_type( sprite_type ).get_sprite_size().y;
    // get the center (topleft coord), then adjust to center the altar_sacrifice_anim, then adjust for altar_sacrifice_anim height
    Cmp::Position new_pos( altar_cmp.getCenter() - sf::Vector2{ 8.f, 4.f } - sf::Vector2{ 0.f, altar_sacrifice_anim_height },
                           Constants::kGridSizePxF );
    m_sound_bank.get_effect( "shrine_lighting" ).play();
    Factory::create_altar_sacrifice_anim( reg(), new_pos, sprite_type );

    m_altar_activation_clock.restart();
  };

  SPDLOG_DEBUG( "Checking altar activation: {}/{}", altar_cmp.get_activation_count(), altar_cmp.get_activation_threshold() );
  // We still need to satisfy the relic sacrifice threshold to get an exitkey drop
  if ( altar_cmp.get_sacrifice_count() < altar_cmp.get_exitkey_drop_threshold() )
  {
    if ( sacrifice_type.contains( "sprite.item.relic" ) )
    {
      switch ( altar_cmp.get_sacrifice_count() )
      {
        case 0: {
          reg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 1 ); } );
          Particle::Factory::add_flame( m_reg, "altar.candle", *altar_uuid_cmp, altar_cmp.position + altar_cmp.flame_offsets[0], 5000 );
          SPDLOG_DEBUG( "Altar activated to state ONE." );
          // Apply the effects from exhuming this item to the player stats
          auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
          for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
          {
            auto &player_stats = Utils::Player::get_player_stats( reg() );
            player_stats.apply_modifiers( inventory_cmp.m_item.actions.at( std::type_index( typeid( Cmp::SacrificeAction ) ) ).action );
          }
          common_activation( SacrificeAnimType::RELIC );
          break;
        }
        case 1: {
          reg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 2 ); } );
          Particle::Factory::add_flame( m_reg, "altar.candle", *altar_uuid_cmp, altar_cmp.position + altar_cmp.flame_offsets[1], 5000 );
          SPDLOG_DEBUG( "Altar activated to state TWO." );
          // Apply the effects from exhuming this item to the player stats
          auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
          for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
          {
            auto &player_stats = Utils::Player::get_player_stats( reg() );
            player_stats.apply_modifiers( inventory_cmp.m_item.actions.at( std::type_index( typeid( Cmp::SacrificeAction ) ) ).action );
          }
          common_activation( SacrificeAnimType::RELIC );
          break;
        }
        case 2: {
          reg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 3 ); } );
          Particle::Factory::add_flame( m_reg, "altar.candle", *altar_uuid_cmp, altar_cmp.position + altar_cmp.flame_offsets[2], 5000 );
          SPDLOG_DEBUG( "Altar activated to state THREE." );
          // Apply the effects from exhuming this item to the player stats
          auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
          for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
          {
            auto &player_stats = Utils::Player::get_player_stats( reg() );
            player_stats.apply_modifiers( inventory_cmp.m_item.actions.at( std::type_index( typeid( Cmp::SacrificeAction ) ) ).action );
          }
          common_activation( SacrificeAnimType::RELIC );
          break;
        }
        case 3: {
          reg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 4 ); } );
          Particle::Factory::add_flame( m_reg, "altar.candle", *altar_uuid_cmp, altar_cmp.position + altar_cmp.flame_offsets[3], 5000 );

          SPDLOG_DEBUG( "Altar activated to state FOUR." );
          // Apply the effects from exhuming this item to the player stats
          auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
          for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
          {
            auto &player_stats = Utils::Player::get_player_stats( reg() );
            player_stats.apply_modifiers( inventory_cmp.m_item.actions.at( std::type_index( typeid( Cmp::SacrificeAction ) ) ).action );
          }
          common_activation( SacrificeAnimType::RELIC );
          break;
        }
      }
    }
  }
  // We still need to satisfy the exitkey sacrifice threshold to get an cryptkey drop
  else if ( altar_cmp.get_sacrifice_count() < altar_cmp.get_cryptkey_drop_threshold() )
  {
    if ( sacrifice_type.contains( "sprite.item.exitkey" ) )
    {
      switch ( altar_cmp.get_sacrifice_count() )
      {
        case 4:
          reg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 5 ); } );
          for ( auto [altar_entt, altar_cmp, altar_uuid_cmp] : reg().view<Cmp::AltarMultiBlock, Cmp::UUID>().each() )
          {
            for ( auto [particle_entt, particle_cmp, particle_uuid_cmp] : reg().view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
            {
              if ( altar_uuid_cmp != particle_uuid_cmp ) continue;
              if ( not particle_cmp.sprite->get_tag().contains( "candle" ) ) continue;
              auto *flame_particle = dynamic_cast<Cmp::Particle::Flame *>( particle_cmp.sprite.get() );
              if ( flame_particle ) { flame_particle->m_final_flame_color = { 0, 0, 255 }; }
            }
          }
          SPDLOG_DEBUG( "Altar activated to state FIVE." );
          common_activation( SacrificeAnimType::KEY );
        default:
          break;
      }
    }
  }

  // We have satisfied the relic sacrifice threshold to get an exitkey drop
  if ( altar_cmp.get_sacrifice_count() == altar_cmp.get_exitkey_drop_threshold() )
  {
    if ( not altar_cmp.is_exitkey_lockout() )
    {
      if ( sacrifice_type.contains( "sprite.item.relic" ) )
      {
        get_systems_event_queue().trigger( Events::CreateItemEvent( Utils::Player::get_position( reg() ), "item.exitkey", "drop_loot" ) );

        // signal UI to flash
        auto flash_entt = reg().create();
        reg().emplace_or_replace<Cmp::FlashUIInventory>( flash_entt );
      }
      SPDLOG_INFO( "Dropped sprite.item.exitkey" );
      altar_cmp.set_exitkey_lockout();
    }
  }
  // We have satisfied the exitkey sacrifice threshold to get an cryptkey drop
  else if ( altar_cmp.get_sacrifice_count() >= altar_cmp.get_cryptkey_drop_threshold() )
  {
    if ( not altar_cmp.is_cryptkey_lockout() )
    {
      entt::entity key_entt = entt::null;

      if ( sacrifice_type.contains( "sprite.item.exitkey" ) )
      {
        get_systems_event_queue().trigger( Events::CreateItemEvent( Utils::Player::get_position( reg() ), "item.cryptkey" ) );
        if ( key_entt != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
        // signal UI to flash
        auto flash_entt = reg().create();
        reg().emplace_or_replace<Cmp::FlashUIInventory>( flash_entt );
      }
      SPDLOG_INFO( "Dropped sprite.item.cryptkey" );
      altar_cmp.set_cryptkey_lockout();
    }
  }
}

} // namespace Game::Sys
