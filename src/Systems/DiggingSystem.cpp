
#include <Events/DropInventoryEvent.hpp>
#include <Player/PlayerNoPath.hpp>
#include <Systems/Stores/ItemStore.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
#include <spdlog/spdlog.h>

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

void DiggingSystem::update( sf::Time dt )
{

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Digging is on cooldown for {} more seconds!", ( digging_cooldown_amount - m_dig_cooldown_clock.getElapsedTime().asSeconds() ) );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
    SPDLOG_DEBUG( "Removing previous Cmp::SelectedPosition {},{} from entity {}", sel_cmp.x, sel_cmp.y, static_cast<int>( existing_sel_entity ) );
  }

  static constexpr float kPlantCheckIntervalHz = 2.0f;
  m_plantcheck_accumulator += dt;
  if ( m_plantcheck_accumulator.asSeconds() >= 1.f / kPlantCheckIntervalHz )
  {
    // check if plant player path blocking should be activated
    auto player_pos = Utils::Player::get_position( reg() );
    for ( auto [plant_entt, plant_cmp, plant_pos_cmp] : reg().view<Cmp::PlantObstacle, Cmp::Position>().each() )
    {
      auto playernopath_cmp = reg().try_get<Cmp::PlayerNoPath>( plant_entt );
      if ( not playernopath_cmp ) continue;

      // enable inactive pathblocking on the plant once the player has moved away from its bbox
      if ( playernopath_cmp->active ) continue;
      if ( not player_pos.findIntersection( plant_pos_cmp ) ) { playernopath_cmp->active = true; }
    }
    m_plantcheck_accumulator = sf::Time::Zero;
  }
}

void DiggingSystem::check_player_smash_pot()
{

  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( not inventory_slot_type.contains( "pickaxe" ) and not inventory_slot_type.contains( "axe" ) and not inventory_slot_type.contains( "shovel" ) )
  {
    return;
  }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  auto loot_container_view = reg().view<Cmp::LootContainer, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [loot_entity, loot_cmp, loot_pos_cmp, loot_anim_cmp] : loot_container_view.each() )
  {
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( loot_pos_cmp ) )
    {
      SPDLOG_INFO( "Found lootable entity at position: [{}, {}]!", loot_pos_cmp.position.x, loot_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
        if ( player_hitbox.findIntersection( loot_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      m_dig_cooldown_clock.restart();
      loot_cmp.hp -= Utils::Maths::to_percent( 100.f, Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value() );

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      if ( loot_cmp.hp > 0 )
      {
        loot_anim_cmp.m_animation_active = true;

        if ( m_sound_bank.get_effect( "hit_pot" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "hit_pot" ).play();
      }
      else
      {
        const std::string selected_type = Sys::ItemStore::instance().get_random_item_from_list(
            { "item.bomb", "item.seeingstone", "item.cursetablet" } );
        SPDLOG_INFO( "Pot revealed {}", selected_type );
        Factory::create_world_item( reg(), loot_pos_cmp, selected_type );

        m_sound_bank.get_effect( "break_pot" ).play();
        auto inventory_wear_view = reg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
        for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
        {
          // Decrease weapons level based on damage dealt
          wear_level.m_level -= Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
          SPDLOG_DEBUG( "Player wear level decreased to {} after digging!", weapons_level.m_level );
        }
        Factory::destroy_loot_container( reg(), loot_entity );
      }
    }
  }
}

void DiggingSystem::check_player_dig_obstacle_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::Obstacle, Cmp::AbsoluteAlpha>( entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, alpha_cmp] : position_view.each() )
  {

    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.position.x, pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
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
      reg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      auto existing_alpha = alpha_cmp.getAlpha();
      auto damage_value = Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value();
      if ( inventory_slot_type.contains( "pickaxe" ) ) {}
      else if ( inventory_slot_type.contains( "shovel" ) or inventory_slot_type.contains( "axe" ) ) { damage_value = damage_value / 10; }
      auto damage_percentage = Utils::Maths::to_percent( 255.f, damage_value );
      auto adjusted_alpha = std::max( 0, existing_alpha - damage_percentage );
      alpha_cmp.setAlpha( adjusted_alpha );

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      if ( alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();

        // replace the obstacle with a detonated component
        Factory::remove_obstacle( reg(), obst_entity );
        Factory::create_detonated( reg(), obst_entity, obst_pos_cmp );

        // add the position to the spatial grid so it can be used in pathfinding
        if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
          pathfinding_navmesh->insert( obst_entity, obst_pos_cmp );

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
  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( inventory_slot_type != "sprite.item.shovel" and inventory_slot_type != "sprite.item.axe" ) { return; }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Still in cooldown" );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::PlantObstacle, Cmp::AbsoluteAlpha>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, alpha_cmp] : position_view.each() )
  {
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
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
      reg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      if ( inventory_slot_type == "sprite.item.shovel" )
      {
        auto existing_alpha = alpha_cmp.getAlpha();
        auto damage_value = Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value();
        auto damage_percentage = Utils::Maths::to_percent( 255.f, damage_value );
        auto adjusted_alpha = std::max( 0, existing_alpha - damage_percentage );
        alpha_cmp.setAlpha( adjusted_alpha );
      }
      else if ( inventory_slot_type == "sprite.item.axe" )
      {
        // axe will instakill all plants
        alpha_cmp.setAlpha( 0 );
      }

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      if ( alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "chopping_final" ).play();
        auto inventory_wear_view = reg().view<Cmp::PlayerInventorySlot>();
        for ( auto [inventory_entt, inventory_slot] : inventory_wear_view.each() )
        {
          if ( inventory_slot.m_item.sprite_type == "sprite.item.shovel" )
          {
            auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
            auto player_pos = Utils::Player::get_position( reg() ).position;
            get_systems_event_queue().trigger( Events::DropInventoryEvent( inventory_entt, player_pos ) );
          }
          else if ( inventory_slot.m_item.sprite_type == "sprite.item.axe" )
          {
            if ( reg().valid( obst_entity ) ) reg().destroy( obst_entity );
          }
        }
        if ( Factory::pickup_world_item( reg(), obst_entity ) == entt::null ) { SPDLOG_INFO( "Could not pick up item" ); }

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