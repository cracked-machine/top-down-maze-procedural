
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPC.hpp>
#include <Components/PlantObstacle.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
#include <Utils/Utils.hpp>
#include <spdlog/spdlog.h>

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/InventoryWearLevel.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

DiggingSystem::DiggingSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&DiggingSystem::on_player_action>( this );
  SPDLOG_DEBUG( "DiggingSystem initialized" );
}

void DiggingSystem::update()
{
  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Digging is on cooldown for {} more seconds!", ( digging_cooldown_amount - m_dig_cooldown_clock.getElapsedTime().asSeconds() ) );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = getReg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    getReg().remove<Cmp::SelectedPosition>( existing_sel_entity );
    SPDLOG_DEBUG( "Removing previous Cmp::SelectedPosition {},{} from entity {}", sel_cmp.x, sel_cmp.y, static_cast<int>( existing_sel_entity ) );
  }
}

void DiggingSystem::check_player_smash_pot()
{

  auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
  if ( not inventory_slot_type.contains( "pickaxe" ) and not inventory_slot_type.contains( "axe" ) and not inventory_slot_type.contains( "shovel" ) )
  {
    return;
  }

  if ( Utils::get_player_inventory_wear_level( getReg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [loot_entity, loot_cmp, loot_pos_cmp, loot_anim_cmp] : loot_container_view.each() )
  {
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::getGameView() );
    if ( mouse_position_bounds.findIntersection( loot_pos_cmp ) )
    {
      SPDLOG_INFO( "Found lootable entity at position: [{}, {}]!", loot_pos_cmp.position.x, loot_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
        if ( player_hitbox.findIntersection( loot_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      m_dig_cooldown_clock.restart();
      loot_cmp.hp -= Utils::to_percent( 100.f, Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingDamagePerHit>( getReg() ).get_value() );

      float reduction_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::reduce_player_inventory_wear_level( getReg(), reduction_amount );

      if ( loot_cmp.hp > 0 )
      {
        loot_anim_cmp.m_animation_active = true;

        if ( m_sound_bank.get_effect( "hit_pot" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "hit_pot" ).play();
      }
      else
      {
        // pot was broken by player
        auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
            std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );

        Factory::createCarryItem( getReg(), loot_pos_cmp, "CARRYITEM.bomb" );

        m_sound_bank.get_effect( "break_pot" ).play();
        auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
        for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
        {
          // Decrease weapons level based on damage dealt
          wear_level.m_level -= Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
          SPDLOG_DEBUG( "Player wear level decreased to {} after digging!", weapons_level.m_level );
        }
        Factory::destroyLootContainer( getReg(), loot_entity );
      }
    }
  }
}

void DiggingSystem::check_player_dig_obstacle_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
  if ( not inventory_slot_type.contains( "pickaxe" ) ) { return; }

  if ( Utils::get_player_inventory_wear_level( getReg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = getReg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    getReg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = getReg().view<Cmp::Position, Cmp::Obstacle, Cmp::AbsoluteAlpha>( entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, alpha_cmp] : position_view.each() )
  {

    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::getGameView() );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.position.x, pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
        if ( player_hitbox.findIntersection( obst_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      getReg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      auto existing_alpha = alpha_cmp.getAlpha();
      auto damage_value = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingDamagePerHit>( getReg() ).get_value();
      auto damage_percentage = Utils::to_percent( 255.f, damage_value );
      auto adjusted_alpha = std::max( 0, existing_alpha - damage_percentage );
      alpha_cmp.setAlpha( adjusted_alpha );

      float reduction_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::reduce_player_inventory_wear_level( getReg(), reduction_amount );

      if ( alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();
        Factory::destroyObstacle( getReg(), obst_entity );
        Factory::createDetonated( getReg(), obst_entity, obst_pos_cmp );
        SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
      }
      else
      {
        // select all pickaxe sounds except the final smash sound
        Cmp::RandomInt random_picker( 1, 6 );
        m_sound_bank.get_effect( "pickaxe" + std::to_string( random_picker.gen() ) ).play();
      }
    }
  }
}

void DiggingSystem::check_player_dig_plant_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
  if ( inventory_slot_type != "CARRYITEM.shovel" and inventory_slot_type != "CARRYITEM.axe" ) { return; }

  if ( Utils::get_player_inventory_wear_level( getReg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Still in cooldown" );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = getReg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    getReg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = getReg().view<Cmp::Position, Cmp::PlantObstacle, Cmp::AbsoluteAlpha>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, alpha_cmp] : position_view.each() )
  {
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::getGameView() );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
        if ( player_hitbox.findIntersection( obst_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      getReg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      if ( inventory_slot_type == "CARRYITEM.shovel" )
      {
        auto existing_alpha = alpha_cmp.getAlpha();
        auto damage_value = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingDamagePerHit>( getReg() ).get_value();
        auto damage_percentage = Utils::to_percent( 255.f, damage_value );
        auto adjusted_alpha = std::max( 0, existing_alpha - damage_percentage );
        alpha_cmp.setAlpha( adjusted_alpha );
      }
      else if ( inventory_slot_type == "CARRYITEM.axe" )
      {
        // axe will instakill all plants
        alpha_cmp.setAlpha( 0 );
      }

      float reduction_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::reduce_player_inventory_wear_level( getReg(), reduction_amount );

      if ( alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "chopping_final" ).play();
        auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot>();
        for ( auto [inventory_entt, inventory_slot] : inventory_wear_view.each() )
        {
          if ( inventory_slot.type == "CARRYITEM.shovel" )
          {
            Factory::dropCarryItem( getReg(), obst_pos_cmp, m_sprite_factory.get_multisprite_by_type( inventory_slot.type ), inventory_entt );
          }
          else if ( inventory_slot.type == "CARRYITEM.axe" )
          {
            if ( getReg().valid( obst_entity ) ) getReg().destroy( obst_entity );
          }
        }
        if ( Factory::pickupCarryItem( getReg(), obst_entity ) == entt::null ) { SPDLOG_INFO( "Could not pick up item" ); }

        SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
      }
      else
      {
        // play digging sound and animation
        m_sound_bank.get_effect( "digging_earth" ).play();
      }
    }
  }
}

void DiggingSystem::check_player_axe_npc_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
  if ( inventory_slot_type != "CARRYITEM.axe" ) { return; }

  if ( Utils::get_player_inventory_wear_level( getReg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Still in cooldown" );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = getReg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    getReg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = getReg().view<Cmp::Position, Cmp::NPC, Cmp::SpriteAnimation>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, anim_cmp] : position_view.each() )
  {
    if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) ) continue;
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::getGameView() );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found NPC entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
        if ( player_hitbox.findIntersection( obst_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      getReg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      float reduction_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::reduce_player_inventory_wear_level( getReg(), reduction_amount );

      // select the final smash sound
      m_sound_bank.get_effect( "axe_whip" ).play();
      m_sound_bank.get_effect( "skele_death" ).play();
      auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot>();
      for ( auto [inventory_entt, inventory_slot] : inventory_wear_view.each() )
      {
        if ( inventory_slot.type == "CARRYITEM.axe" )
        {
          if ( getReg().valid( obst_entity ) ) getReg().destroy( obst_entity );
        }
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
    }
  }
}

void DiggingSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_dig_obstacle_collision();
    check_player_dig_plant_collision();
    check_player_smash_pot();
    check_player_axe_npc_collision();
  }
}

} // namespace ProceduralMaze::Sys