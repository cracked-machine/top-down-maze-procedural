#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/Altar/MultiBlock.hpp>
#include <Components/Altar/Sacrifice.hpp>
#include <Components/Altar/Segment.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Inventory/FlashUIHealth.hpp>
#include <Components/Inventory/FlashUIInventory.hpp>
#include <Components/Inventory/FlashUIWealth.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Particle/Flame.hpp>
#include <Components/Player/AltarActivationCooldown.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Player/ExtraLife.hpp>
#include <Components/Player/KeysCount.hpp>
#include <Components/Player/Wealth.hpp>
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
#include <SFML/System/Time.hpp>
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

  auto altar_view = reg().view<Cmp::Altar::MultiBlock>();
  auto player_hitbox = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ).position, Constants::kGridSizePxF, 1.5f );

  for ( auto [altar_entity, altar_cmp] : altar_view.each() )
  {
    if ( not player_hitbox.findIntersection( altar_cmp ) ) continue;

    SPDLOG_DEBUG( "Player collided with Altar at ({}, {})", altar_cmp.position.x, altar_cmp.position.y );
    check_player_altar_activation( altar_entity, altar_cmp );
  }
}

void AltarSystem::update()
{
  // tidy up any dead altar sacrifice animations
  auto altar_sacrifice_view = reg().view<Cmp::Altar::Sacrifice, Cmp::AnimData>();
  for ( auto [altar_sacrifice_entt, altar_sacrifice_cmp, altar_sacrifice_anim_cmp] : altar_sacrifice_view.each() )
  {
    if ( not altar_sacrifice_anim_cmp.m_enabled )
    {
      if ( reg().valid( altar_sacrifice_entt ) ) { reg().destroy( altar_sacrifice_entt ); }
    }
  }
}

void AltarSystem::check_player_altar_activation( entt::entity altar_entity, Cmp::Altar::MultiBlock &altar_cmp )
{
  static const sf::Time kActivationCooldownSeconds{ sf::seconds( 3.f ) };
  auto *altar_cooldown = reg().try_get<Cmp::Player::AltarActivationCooldown>( Utils::Player::get_entity( reg() ) );
  if ( altar_cooldown and altar_cooldown->getElapsedTime() < kActivationCooldownSeconds ) return;

  auto *altar_uuid_cmp = reg().try_get<Cmp::UUID>( altar_entity );
  if ( not altar_uuid_cmp ) throw std::runtime_error( "Altar does not have UUID" );

  auto [sacrifice_entt, sacrifice_type] = Utils::Player::get_inventory_type( reg() );
  enum class SacrificeAnimType { RELIC, KEY, JEWELS, WITCHESJAR };

  //! @brief Common actions following an altar sacrifice
  auto common_activation = [&]( SacrificeAnimType sacrifice_anim_type )
  {
    Sprites::SpriteMetaType sprite_type = ( sacrifice_anim_type == SacrificeAnimType::KEY ) ? "sprite.graveyard.altar.key.anim"
                                                                                            : "sprite.graveyard.altar.relic.anim";

    Factory::Player::destroy_inventory( reg(), sacrifice_type );

    float altar_sacrifice_anim_height = m_sprite_factory.get_spritesheet_by_type( sprite_type ).get_sprite_size().y;
    // get the center (topleft coord), then adjust to center the altar_sacrifice_anim, then adjust for altar_sacrifice_anim height
    Cmp::Position new_pos( altar_cmp.getCenter() - sf::Vector2{ 8.f, 4.f } - sf::Vector2{ 0.f, altar_sacrifice_anim_height },
                           Constants::kGridSizePxF );
    m_sound_bank.get_effect( "shrine_lighting" ).play();
    Factory::Altar::create_altar_sacrifice_anim( reg(), new_pos, sprite_type );

    reg().emplace_or_replace<Cmp::Player::AltarActivationCooldown>( Utils::Player::get_entity( reg() ) );
  };

  // sacrifice jewels at any time
  if ( sacrifice_type.contains( "item.jewelry" ) )
  {
    SPDLOG_INFO( "Player made an offering: {}", sacrifice_type );
    common_activation( SacrificeAnimType::JEWELS );
    auto &player_wealth = Utils::Player::get_wealth( reg() );
    player_wealth.wealth += 2;
    // signal UI to flash
    auto flash_entt = reg().create();
    reg().emplace_or_replace<Cmp::FlashUIWealth>( flash_entt );
  }

  // sacrifice witches jar at any time
  if ( sacrifice_type.contains( "item.witchesjar" ) )
  {
    auto *anim_cmp = reg().try_get<Cmp::AnimData>( altar_entity );
    if ( not anim_cmp ) return;
    SPDLOG_INFO( "Player made an offering: {}", sacrifice_type );

    if ( anim_cmp->m_sprite_type == "sprite.crypt.altar.inactive" )
    {
      if ( not Utils::Player::get_projected_position( reg() ).findIntersection( altar_cmp ) ) return;

      // player gains extra life protection
      anim_cmp->m_sprite_type = "sprite.crypt.altar.active";
      Factory::Particle::add_crypt_altar_ps( m_reg, "crypt.altar.particles", 0.5, 25.f, *altar_uuid_cmp,
                                             { altar_cmp.position.x + 8.f, altar_cmp.position.y + 24.f }, 5000 );
      Factory::Player::destroy_inventory( reg(), sacrifice_type );
      auto flash_entt = reg().create();
      reg().emplace_or_replace<Cmp::FlashUIHealth>( flash_entt );
      reg().emplace_or_replace<Cmp::Player::ExtraLife>( Utils::Player::get_entity( reg() ) );
      m_sound_bank.get_effect( "crypt_altar_activate" ).play();
    }
    else if ( anim_cmp->m_sprite_type.contains( "sprite.graveyard.altar" ) )
    {
      // player kills all spawned ghosts in the game
      common_activation( SacrificeAnimType::WITCHESJAR );
      for ( auto [npc_entt, npc_cmp, npc_pos_cmp, anim_cmp] : reg().view<Cmp::Npc::NPC, Cmp::Position, Cmp::AnimData>().each() )
      {
        if ( anim_cmp.m_sprite_type.contains( "ghost" ) )
        {
          Factory::Npc::create_npc_death_anim( reg(), npc_pos_cmp, "sprite.death.anim.banish" );
          Factory::Npc::destroy_npc( reg(), npc_entt );
        }
      }
    }
    m_sound_bank.get_effect( "witches_jar_sacrifice" ).play();
  }

  // We still need to satisfy the relic sacrifice threshold to get an exitkey drop
  if ( altar_cmp.get_sacrifice_count() < altar_cmp.get_exitkey_drop_threshold() )
  {
    if ( sacrifice_type.contains( "sprite.item.relic" ) )
    {
      uint8_t sacrifice_count = altar_cmp.get_sacrifice_count();
      reg().patch<Cmp::Altar::MultiBlock>( altar_entity,
                                           [&]( Cmp::Altar::MultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( sacrifice_count + 1 ); } );
      Factory::Particle::add_flame( m_reg, "altar.candle", *altar_uuid_cmp, altar_cmp.position + altar_cmp.flame_offsets[sacrifice_count], 5000 );
      SPDLOG_DEBUG( "Altar activated to state {}.", sacrifice_count + 1 );

      // Apply the effects from exhuming this item to the player stats
      auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
      for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
      {
        auto &player_stats = Utils::Player::get_player_stats( reg() );
        player_stats.apply_modifiers( inventory_cmp.m_item.actions.at( std::type_index( typeid( Cmp::SacrificeAction ) ) ).action );
      }
      common_activation( SacrificeAnimType::RELIC );
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
          reg().patch<Cmp::Altar::MultiBlock>( altar_entity, [&]( Cmp::Altar::MultiBlock &altar_cmp ) { altar_cmp.set_sacrifice_count( 5 ); } );
          for ( auto [altar_entt, altar_cmp, altar_uuid_cmp] : reg().view<Cmp::Altar::MultiBlock, Cmp::UUID>().each() )
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
      if ( sacrifice_type.contains( "sprite.item.exitkey" ) )
      {
        get_systems_event_queue().trigger( Events::CreateItemEvent( Utils::Player::get_position( reg() ), "item.cryptkey", "drop_loot" ) );
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
