
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LootContainer.hpp>
#include <Components/PlantObstacle.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
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
  auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
  bool has_tool = false;
  for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
  {
    if ( inventory_cmp.type == Cmp::CarryItemType::PICKAXE or inventory_cmp.type == Cmp::CarryItemType::AXE or
         inventory_cmp.type == Cmp::CarryItemType::SHOVEL )
    {
      has_tool = true;
    }
  }
  if ( not has_tool ) { return; }

  auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    if ( wear_level.m_level <= 0 )
    {
      SPDLOG_DEBUG( "Player weapons level is {}, cannot dig!", weapons_level.m_level );
      return;
    }
  }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  // detonate loot containers - component removal is handled by LootSystem
  auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position>();
  for ( auto [loot_entity, loot_cmp, loot_pos_cmp] : loot_container_view.each() )
  {
    // Remap the mouse position to game view coordinates (a subset of the actual game area)
    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
    sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

    // Check if the mouse position intersects with the entity's position
    auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );
    if ( mouse_position_bounds.findIntersection( loot_pos_cmp ) )
    {
      SPDLOG_INFO( "Found lootable entity at position: [{}, {}]!", loot_pos_cmp.position.x, loot_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_view.each() )
      {
        auto half_sprite_size = Constants::kGridSquareSizePixelsF;
        auto player_horizontal_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::HORIZONTAL );
        auto player_vertical_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::VERTICAL );
        if ( player_horizontal_bounds.findIntersection( loot_pos_cmp ) || player_vertical_bounds.findIntersection( loot_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby )
      {
        SPDLOG_INFO( " Player not close enough to loot at position ({}, {})!", loot_pos_cmp.position.x, loot_pos_cmp.position.y );
        continue;
      }

      auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );

      // // clang-format off
      // [[maybe_unused]] auto loot_entt = Factory::createLootDrop(
      //   getReg(),
      //   Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),
      //   sf::FloatRect{ loot_pos_cmp.position, loot_pos_cmp.size },
      //   Factory::IncludePack<>{},
      //   Factory::ExcludePack<>{} );
      // // clang-format on

      Factory::createCarryItem( getReg(), loot_pos_cmp, Cmp::CarryItemType::BOMB );

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

void DiggingSystem::check_player_dig_obstacle_collision()
{
  auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
  bool has_pickaxe = false;
  for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
  {
    if ( inventory_cmp.type == Cmp::CarryItemType::PICKAXE ) { has_pickaxe = true; }
  }
  if ( not has_pickaxe ) { return; }

  auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    if ( wear_level.m_level <= 0 )
    {
      SPDLOG_DEBUG( "Player weapons level is {}, cannot dig!", weapons_level.m_level );
      return;
    }
  }

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

    // Remap the mouse position to game view coordinates (a subset of the actual game area)
    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
    sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

    // Check if the mouse position intersects with the entity's position
    auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.position.x, pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_view.each() )
      {
        auto half_sprite_size = Constants::kGridSquareSizePixelsF;
        auto player_horizontal_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::HORIZONTAL );
        auto player_vertical_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::VERTICAL );
        if ( player_horizontal_bounds.findIntersection( obst_pos_cmp ) || player_vertical_bounds.findIntersection( obst_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby )
      {
        SPDLOG_DEBUG( " Player not close enough to dig at position ({}, {})!", pos_cmp.position.x, pos_cmp.position.y );
        continue;
      }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      getReg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      auto damage_per_dig = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingDamagePerHit>( getReg() ).get_value();
      alpha_cmp.setAlpha( std::max( 0, alpha_cmp.getAlpha() - damage_per_dig ) );
      SPDLOG_DEBUG( "Applied {} digging damage to obstacle at position ({}, {}), new alpha is {}.", damage_per_dig, obst_pos_cmp.position.x,
                    obst_pos_cmp.position.y, alpha_cmp.getAlpha() );

      auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
      for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
      {
        // Decrease weapons level based on damage dealt
        wear_level.m_level -= Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
        SPDLOG_DEBUG( "Player wear level decreased to {} after digging!", weapons_level.m_level );
      }

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
  auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
  bool has_shovel = false;
  for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
  {
    if ( inventory_cmp.type == Cmp::CarryItemType::SHOVEL ) { has_shovel = true; }
  }
  if ( not has_shovel )
  {
    SPDLOG_DEBUG( "Player does not have shovel" );
    return;
  }

  auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [inventory_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    if ( wear_level.m_level <= 0 )
    {
      SPDLOG_DEBUG( "Player shovel level is {}, cannot dig!", wear_level.m_level );
      return;
    }
  }

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

    // Remap the mouse position to game view coordinates (a subset of the actual game area)
    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
    sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

    // Check if the mouse position intersects with the entity's position
    auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_view.each() )
      {
        auto half_sprite_size = Constants::kGridSquareSizePixelsF;
        auto player_horizontal_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::HORIZONTAL );
        auto player_vertical_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f, Cmp::RectBounds::ScaleCardinality::VERTICAL );
        if ( player_horizontal_bounds.findIntersection( obst_pos_cmp ) || player_vertical_bounds.findIntersection( obst_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby )
      {
        SPDLOG_DEBUG( " Player not close enough to dig at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
        continue;
      }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      getReg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      auto damage_per_dig = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DiggingDamagePerHit>( getReg() ).get_value();
      alpha_cmp.setAlpha( std::max( 0, alpha_cmp.getAlpha() - damage_per_dig ) );
      SPDLOG_DEBUG( "Applied {} digging damage to obstacle at position ({}, {}), new alpha is {}.", damage_per_dig, obst_pos_cmp.position.x,
                    obst_pos_cmp.position.y, alpha_cmp.getAlpha() );

      auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
      for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
      {
        // Decrease weapons level based on damage dealt
        wear_level.m_level -= Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
        SPDLOG_DEBUG( "Player wear level decreased to {} after digging!", wear_level.m_level );
      }

      if ( alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();
        auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot>();
        for ( auto [inventory_entt, inventory_slot] : inventory_wear_view.each() )
        {
          if ( inventory_slot.type == Cmp::CarryItemType::SHOVEL )
          {
            Factory::dropCarryItem( getReg(), obst_pos_cmp, m_sprite_factory.get_multisprite_by_type( "INVENTORY" ), inventory_entt );
          }
        }
        if ( Factory::pickupCarryItem( getReg(), obst_entity ) == entt::null ) { SPDLOG_INFO( "Could not pick up item" ); }

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

void DiggingSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_dig_obstacle_collision();
    check_player_dig_plant_collision();
    check_player_smash_pot();
  }
}

} // namespace ProceduralMaze::Sys