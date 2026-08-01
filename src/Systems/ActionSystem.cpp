
#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Direction.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/LastDirection.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Moveable.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Systems/ActionSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
#include <spdlog/spdlog.h>

namespace Game::Sys
{

ActionSystem::ActionSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&ActionSystem::on_player_action>( this );
  SPDLOG_DEBUG( "ActionSystem initialized" );
}

void ActionSystem::update( [[maybe_unused]] sf::Time dt )
{

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Digging is on cooldown for {} more seconds!", ( digging_cooldown_amount - m_dig_cooldown_clock.getElapsedTime().asSeconds() ) );
    return;
  }
}

void ActionSystem::check_player_smash_pot()
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

  auto loot_container_view = reg().view<Cmp::LootContainer, Cmp::Position, Cmp::AnimData>();
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
        loot_anim_cmp.m_enabled = true;

        if ( m_sound_bank.get_effect( "hit_pot" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "hit_pot" ).play();
      }
      else
      {
        const std::string selected_type = Sys::ItemStore::instance().get_random_item_from_list(
            { "item.bomb", "item.seeingstone", "item.cursetablet" } );
        SPDLOG_INFO( "Pot revealed {}", selected_type );

        get_systems_event_queue().trigger( Events::CreateItemEvent( Utils::Player::get_position( reg() ), selected_type, "drop_loot" ) );

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

void ActionSystem::select_moveable_obstacle()
{
  auto position_view = reg().view<Cmp::Position, Cmp::Obstacle, Cmp::Moveable>( entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, move_cmp] : position_view.each() )
  {
    // project one grdi position in the direction that the player is currently facing to find the obstacle selection
    auto player_pos = Utils::Player::get_position( reg() );
    auto player_last_direction = Utils::Player::get_last_direction( reg() );
    Cmp::Position selected_position( { player_pos.getCenter().x + ( player_last_direction.x * Constants::kGridSizePxF.x ),
                                       player_pos.getCenter().y + ( player_last_direction.y * Constants::kGridSizePxF.y ) },
                                     { 1.f, 1.f } );

    if ( selected_position.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_INFO( "Found moveable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
      reg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );
    }
  }
}

void ActionSystem::deselect_all_moveable_obstacles()
{
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
    SPDLOG_DEBUG( "Removing previous Cmp::SelectedPosition {},{} from entity {}", sel_cmp.x, sel_cmp.y, static_cast<int>( existing_sel_entity ) );
  }
}

void ActionSystem::check_player_dig_obstacle_collision()
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
  auto position_view = reg().view<Cmp::Position, Cmp::Obstacle, Cmp::AbsoluteAlpha, Cmp::AnimData, Cmp::UUID>(
      entt::exclude<Cmp::SelectedPosition> );
  for ( auto [obstacle_entt, obstacle_pos_cmp, obstacle_cmp, obstacle_alpha_cmp, obstacle_anim_cmp, obstacle_uuid_cmp] : position_view.each() )
  {
    if ( not obstacle_anim_cmp.m_sprite_type.contains( ".main" ) ) continue;

    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( obstacle_pos_cmp ) )
    {
      // Reserved obstacles sit under structures and cannot be dug, with one exception:
      // a replanted plant reserves the tiles it lands on, but must not shield the
      // obstacle underneath it from digging.
      if ( reg().all_of<Cmp::ReservedPosition>( obstacle_entt ) )
      {
        bool reserved_by_plant = false;
        for ( auto [seg_entt, seg_cmp, seg_pos_cmp] : reg().view<Cmp::PlantSegment, Cmp::Position>().each() )
        {
          if ( seg_pos_cmp.findIntersection( obstacle_pos_cmp ) )
          {
            reserved_by_plant = true;
            break;
          }
        }
        if ( not reserved_by_plant ) continue;
      }

      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.position.x, pos_cmp.position.y );

      auto player_pos_cmp = Utils::Player::get_position( reg() );

      // check player is near obstacle that was mouse-selected
      auto player_hitbox = Cmp::RectBounds::scaled( player_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
      if ( not player_hitbox.findIntersection( obstacle_pos_cmp ) ) continue;

      // check player is facing the obstacle
      auto player_last_direction = Utils::Player::get_last_direction( reg() );
      Cmp::Position player_projected_position(
          { Utils::Player::get_position( reg() ).getCenter().x + ( player_last_direction.x * Constants::kGridSizePxF.x ),
            Utils::Player::get_position( reg() ).getCenter().y + ( player_last_direction.y * Constants::kGridSizePxF.y ) },
          { 1.f, 1.f } );
      if ( not player_projected_position.findIntersection( obstacle_pos_cmp ) ) continue;

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( obstacle_entt, obstacle_pos_cmp.position );

      m_dig_cooldown_clock.restart();

      // calculate new alpha value and apply to the current obstacle and any obstacle with matching UUID (cap sprite obstacles)
      auto damage_per_hit = Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value();
      if ( inventory_slot_type.contains( "pickaxe" ) ) { /* no damage gradient for pickaxe */ }
      else if ( inventory_slot_type.contains( "shovel" ) or inventory_slot_type.contains( "axe" ) ) { damage_per_hit = damage_per_hit / 10; }
      auto new_alpha_value = std::max( 0, obstacle_alpha_cmp.getAlpha() - Utils::Maths::to_percent( 255.f, damage_per_hit ) );
      obstacle_alpha_cmp.setAlpha( new_alpha_value );
      auto cap_obstacle_view = reg().view<Cmp::Obstacle, Cmp::UUID, Cmp::AbsoluteAlpha, Cmp::Position>();
      for ( auto [cap_obstacle_entt, cap_obstacle_cmp, cap_obstacle_uuid_cmp, cap_obstacle_alpha, cap_pos_cmp] : cap_obstacle_view.each() )
      {
        if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), cap_pos_cmp ) ) continue;
        if ( obstacle_uuid_cmp != cap_obstacle_uuid_cmp ) continue;
        cap_obstacle_alpha.setAlpha( new_alpha_value );
      }

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      if ( obstacle_alpha_cmp.getAlpha() == 0 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();

        // replace the obstacle with a detonated component
        Factory::remove_obstacle( reg(), obstacle_entt, Factory::DeleteExtras::Yes );
        Factory::add_detonated( reg(), obstacle_entt, obstacle_pos_cmp );

        // add the position to the spatial grid so it can be used in pathfinding
        if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_npc_navmesh.lock() )
          pathfinding_navmesh->insert( obstacle_entt, obstacle_pos_cmp );
        if ( PathFinding::SpatialHashGridSharedPtr ghost_navmesh = m_ghost_navmesh.lock() )
          ghost_navmesh->insert( obstacle_entt, obstacle_pos_cmp );
        if ( PathFinding::SpatialHashGridSharedPtr player_navmesh = m_player_navmesh.lock() )
          player_navmesh->insert( obstacle_entt, obstacle_pos_cmp );

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

void ActionSystem::check_player_dig_plant_collision()
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
  auto position_view = reg().view<Cmp::PlantMultiBlock, Cmp::UUID>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [plant_entt, plant_mb_cmp, plant_uuid_cmp] : position_view.each() )
  {
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( plant_mb_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
        if ( player_hitbox.findIntersection( plant_mb_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( plant_entt, plant_mb_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

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
          Factory::remove_plant_mb( reg(), plant_entt, m_npc_navmesh.lock(), m_player_navmesh.lock() );
        }
      }
      get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG, plant_entt ) );

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
    }
  }
}

void ActionSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_dig_obstacle_collision();
    check_player_dig_plant_collision();
    check_player_smash_pot();
  }
  else if ( event.action == Events::PlayerActionEvent::GameActions::SELECT ) { select_moveable_obstacle(); }
  else if ( event.action == Events::PlayerActionEvent::GameActions::DESELECT ) { deselect_all_moveable_obstacles(); }
}

} // namespace Game::Sys