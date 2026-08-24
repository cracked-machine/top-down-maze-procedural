
#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Crypt/Chest.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Direction.hpp>
#include <Components/FractalCurve.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Inventory/Explosive.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Inventory/WearLevel.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/LastDirection.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Moveable.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Obstacle.hpp>
#include <Components/ObstacleCap.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Player/DiggingCooldown.hpp>
#include <Components/Player/NoPath.hpp>
#include <Components/Random.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/Stats/CarryAction.hpp>
#include <Components/Stats/DestroyAction.hpp>
#include <Components/Stats/SpawnAction.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/ParticleFactory.hpp>
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
#include <Systems/Threats/LightningSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
#include <numbers>
#include <source_location>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>

namespace Game::Sys
{

ActionSystem::ActionSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&ActionSystem::on_player_action>( this );
  std::ignore = get_systems_event_queue().sink<Events::DropInventoryEvent>().connect<&ActionSystem::on_drop_inventory_event>( this );
  SPDLOG_DEBUG( "ActionSystem initialized" );
}

void ActionSystem::update( [[maybe_unused]] sf::Time dt )
{
  // destroy particle sprite entities once all their particles have expired
  Factory::Particle::delete_expired_particle_sprites( reg(), "graveyard.obstacle.dig.particle" );
  Factory::Particle::delete_expired_particle_sprites( reg(), "graveyard.plant.leaves.particle" );
  Factory::Particle::delete_expired_particle_sprites( reg(), "graveyard.plant.twigs.particle" );

  // abort if still in cooldown
  if ( is_digging_on_cooldown() ) { return; }
}

void ActionSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DROP_CARRYITEM )
  {
    // if player is standing next to a Cmp::Crypt::Chest let them open it without dropping the inventory item
    for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : reg().view<Cmp::Crypt::Chest, Cmp::Position>().each() )
    {
      if ( not Utils::Player::is_player_near( reg(), chest_pos_cmp ) ) continue;
      return;
    }

    if ( m_inventory_cooldown_timer.getElapsedTime() < sf::milliseconds( 750.f ) ) return;

    auto player_pos = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ), 0.5 );
    Sprites::SpriteMetaType existing_player_inventory_type;

    // drop inventory if we have one
    auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
    for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
    {
      existing_player_inventory_type = inventory_cmp.m_item.sprite_type;
      drop_inventory_item( Utils::Player::get_position( reg() ).position, inventory_entt );
    }

    // pickup inventory if there is something at this position
    auto world_carryitem_view = reg().view<Cmp::WorldItem, Cmp::Position>();
    for ( auto [carryitem_entt, carryitem_cmp, pos_cmp] : world_carryitem_view.each() )
    {
      if ( not player_pos.findIntersection( pos_cmp ) ) continue;                  // is there something to pick up?
      if ( carryitem_cmp.sprite_type == existing_player_inventory_type ) continue; // dont pick up the one we just dropped
      if ( inventory_view.size() > 0 ) { break; }                                  // don't pickup another if we already have one

      // ok pick it up
      SPDLOG_DEBUG( "GameActions::DROP_CARRYITEM calling 'pickup_world_item' with entt id {} ", static_cast<uint32_t>( carryitem_entt ) );
      pickup_world_item( reg(), carryitem_entt );
    }
    m_inventory_cooldown_timer.restart();
    SPDLOG_DEBUG( "inventory_view: {} ", inventory_view.size() );
  }
  else if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_dig_obstacle_collision();
    check_player_dig_plant_collision();
    check_player_smash_pot();
  }
  else if ( event.action == Game::Events::PlayerActionEvent::GameActions::ATTACK )
  {
    // axe attack?!
    check_player_axe_npc_kill();
  }
  else if ( event.action == Events::PlayerActionEvent::GameActions::SELECT ) { select_moveable_obstacle(); }
  else if ( event.action == Events::PlayerActionEvent::GameActions::DESELECT ) { reset_all_selected_positions(); }
}

void ActionSystem::on_drop_inventory_event( Game::Events::DropInventoryEvent ev ) { drop_inventory_item( ev.drop_pos, ev.inventory_slot_entt ); }

bool ActionSystem::is_digging_on_cooldown()
{
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  auto *player_dig_cooldown = reg().try_get<Cmp::Player::DiggingCooldown>( Utils::Player::get_entity( reg() ) );
  return ( player_dig_cooldown != nullptr ) and player_dig_cooldown->getElapsedTime() < sf::seconds( digging_cooldown_amount );
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
  if ( is_digging_on_cooldown() ) { return; }

  auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
  auto loot_container_view = reg().view<Cmp::LootContainer, Cmp::Position, Cmp::AnimData>();
  for ( auto [loot_entity, loot_container, loot_container_pos, loot_container_anim] : loot_container_view.each() )
  {
    if ( mouse_position_bounds.findIntersection( loot_container_pos ) )
    {
      SPDLOG_INFO( "Found lootable entity at position: [{}, {}]!", loot_container_pos.position.x, loot_container_pos.position.y );

      // check player is near obstacle that was mouse-selected
      if ( not Utils::Player::is_player_near( reg(), loot_container_pos ) ) continue;

      // check player is facing the obstacle
      if ( not Utils::Player::get_projected_position( reg() ).findIntersection( loot_container_pos ) ) continue;

      reg().emplace_or_replace<Cmp::Player::DiggingCooldown>( Utils::Player::get_entity( reg() ) );
      loot_container.hp -= Utils::Maths::to_percent( 100.f, Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value() );

      float weapon_dmg_delta = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), weapon_dmg_delta );

      if ( loot_container.hp > 0 )
      {
        loot_container_anim.m_enabled = true;

        if ( m_sound_bank.get_effect( "hit_pot" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "hit_pot" ).play();
      }
      else
      {
        const std::string selected_type = Sys::ItemStore::instance().get_random_item_from_list(
            Utils::Player::get_player_stats( reg() ).luck(), { "item.cursetablet", "item.seeingstone", "item.bomb" } );

        get_systems_event_queue().trigger( Events::CreateItemEvent( Utils::Player::get_position( reg() ), selected_type, "drop_loot" ) );

        m_sound_bank.get_effect( "break_pot" ).play();
        auto inventory_wear_view = reg().view<Cmp::PlayerInventorySlot, Cmp::Inventory::WearLevel>();
        for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
        {
          // Decrease weapons level based on damage dealt
          wear_level.m_level -= Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
        }
        Factory::Loot::destroy_loot_container( reg(), loot_entity );
      }
    }
  }
}

void ActionSystem::check_player_axe_npc_kill()
{
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_npc_navmesh.lock();
  if ( not pathfinding_navmesh ) return;

  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( inventory_slot_type != "sprite.item.axe" ) { return; }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // Remove any existing SelectedPosition from NPCs only — this runs every frame the attack button is
  // held with no cooldown of its own, so clearing the whole registry's SelectedPosition here would
  // also wipe unrelated selections (e.g. the obstacle currently being dug) set by other systems
  auto selected_position_view = reg().view<Cmp::SelectedPosition, Cmp::Npc::NPC>();
  for ( auto [existing_sel_entity, sel_cmp, npc_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::Npc::NPC, Cmp::AnimData>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [npc_entity, npc_pos_cmp, npc_cmp, anim_cmp] : position_view.each() )
  {
    if ( anim_cmp.m_sprite_type.contains( "sprite.ghost" ) ) continue;
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( npc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found NPC entity at position: [{}, {}]!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::Player::Character, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
        if ( player_hitbox.findIntersection( npc_pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( npc_entity, npc_pos_cmp.position );

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      // select the final smash sound
      m_sound_bank.get_effect( "axe_whip" ).play();
      m_sound_bank.get_effect( "rattling_bones" ).play();

      auto skelebones_particle_uuid = Cmp::UUID::generate();
      Factory::Particle::add_skelebones_ps( reg(), "graveyard.skele.bones.particle", 50, 2.f, 50.f, 14.f, skelebones_particle_uuid,
                                            npc_pos_cmp.getCenter(), npc_pos_cmp.position.y );
      // drop loot - 1 in 3 chance
      auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "sprite.graveyard.loot.health", "sprite.graveyard.loot.blast", "sprite.graveyard.loot.repair" } );

      Cmp::RandomInt do_drop( 0, 2 );
      if ( do_drop.gen() == 0 )
      {
        auto dropped_loot_entt = Factory::Loot::create_loot_drop(
            reg(), Cmp::AnimData( Cmp::AnimData::Config{ .sprite_type = sprite_type, .enabled = false } ),
            Cmp::RectBounds::scaled( npc_pos_cmp.position, npc_pos_cmp.size, 2.f ).getBounds(), Factory::IncludePack<>{},
            Factory::ExcludePack<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Obstacle>{},
            Factory::ExcludePack<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Obstacle>{} );

        if ( dropped_loot_entt != entt::null )
        {
          auto player_pos = Utils::Player::get_position( reg() );
          SPDLOG_INFO( "Player position was at {},{} when loot was dropped", player_pos.position.x, player_pos.position.y );
          m_sound_bank.get_effect( "drop_loot" ).play();
        }
      }

      // now destroy the NPC
      if ( reg().valid( npc_entity ) )
      {
        pathfinding_navmesh->remove( npc_entity, npc_pos_cmp );
        Factory::Npc::destroy_npc( reg(), npc_entity );
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
    }
  }
}

void ActionSystem::select_moveable_obstacle()
{
  // project one grid position in the direction that the player is currently facing to find the obstacle selection
  auto selected_position = Utils::Player::get_projected_position( reg() );

  auto position_view = reg().view<Cmp::Position, Cmp::Obstacle, Cmp::Moveable>( entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );
  for ( auto [obst_entity, obst_pos_cmp, obst_cmp, move_cmp] : position_view.each() )
  {
    if ( selected_position.findIntersection( obst_pos_cmp ) )
    {
      SPDLOG_INFO( "Found moveable entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
      reg().emplace_or_replace<Cmp::SelectedPosition>( obst_entity, obst_pos_cmp.position );
    }
  }
}

void ActionSystem::reset_all_selected_positions()
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
  if ( is_digging_on_cooldown() ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  reset_all_selected_positions();

  // Cap position lookup by UUID, built once so each obstacle below can find its paired cap in O(1)
  // rather than rescanning every cap entity in the level.
  std::unordered_map<Cmp::UUID, Cmp::Position> cap_position_by_uuid;
  for ( auto [cap_entt, cap_cmp, cap_pos_cmp, cap_uuid_cmp] : reg().view<Cmp::ObstacleCap, Cmp::Position, Cmp::UUID>().each() )
  {
    cap_position_by_uuid.emplace( cap_uuid_cmp, cap_pos_cmp );
  }

  auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::Obstacle, Cmp::AbsoluteAlpha, Cmp::AnimData, Cmp::UUID>( entt::exclude<Cmp::SelectedPosition> );
  for ( auto [obstacle_entt, obstacle_pos_cmp, obstacle_cmp, obstacle_alpha_cmp, obstacle_anim_cmp, obstacle_uuid_cmp] : position_view.each() )
  {
    if ( not obstacle_anim_cmp.m_sprite_type.contains( ".main" ) ) continue;

    // The obstacle's cap is rendered as a separate entity directly above it, and from the
    // front-facing view the player sees obstacle and cap as one continuous shape - so a click on
    // either must select the same obstacle. The cap is matched strictly by its paired UUID (never
    // by position), so a different obstacle stacked above - whose own cap may visually overlap this
    // one - can never be picked up here.
    bool mouse_over_obstacle_or_cap = static_cast<bool>( mouse_position_bounds.findIntersection( obstacle_pos_cmp ) );
    if ( not mouse_over_obstacle_or_cap )
    {
      auto cap_it = cap_position_by_uuid.find( obstacle_uuid_cmp );
      if ( cap_it != cap_position_by_uuid.end() )
      {
        mouse_over_obstacle_or_cap = static_cast<bool>( mouse_position_bounds.findIntersection( cap_it->second ) );
      }
    }

    if ( mouse_over_obstacle_or_cap )
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

      // check player is near obstacle that was mouse-selected
      if ( not Utils::Player::is_player_near( reg(), obstacle_pos_cmp ) ) continue;

      // check player is facing the obstacle
      if ( not Utils::Player::get_projected_position( reg() ).findIntersection( obstacle_pos_cmp ) ) continue;

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( obstacle_entt, obstacle_pos_cmp.position );

      reg().emplace_or_replace<Cmp::Player::DiggingCooldown>( Utils::Player::get_entity( reg() ) );

      // calculate new alpha value and apply to the current obstacle and any obstacle with matching UUID (cap sprite obstacles)
      auto damage_per_hit = Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value();
      if ( inventory_slot_type.contains( "pickaxe" ) ) { damage_per_hit = damage_per_hit / 2; }
      else if ( inventory_slot_type.contains( "shovel" ) or inventory_slot_type.contains( "axe" ) ) { damage_per_hit = damage_per_hit / 5; }
      obstacle_cmp.damage += damage_per_hit;

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      // Destroy the obstacle?
      if ( obstacle_cmp.damage >= 100 )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();

        // replace the obstacle with a detonated component
        Factory::Obstacle::remove_obstacle( reg(), obstacle_entt, Factory::Obstacle::DeleteExtras::Yes );
        for ( auto [ob_crack_entt, ob_crack_cmp, ob_crack_uuid] : reg().view<Cmp::ObstacleCrack, Cmp::UUID>().each() )
        {
          if ( ob_crack_uuid == obstacle_uuid_cmp ) reg().destroy( ob_crack_entt );
        }
        Factory::Bomb::add_detonated( reg(), obstacle_entt, obstacle_pos_cmp );

        // add the position to the spatial grid so it can be used in pathfinding
        if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_npc_navmesh.lock() )
          pathfinding_navmesh->insert( obstacle_entt, obstacle_pos_cmp );
        if ( PathFinding::SpatialHashGridSharedPtr ghost_navmesh = m_ghost_navmesh.lock() ) ghost_navmesh->insert( obstacle_entt, obstacle_pos_cmp );
        if ( PathFinding::SpatialHashGridSharedPtr player_navmesh = m_player_navmesh.lock() )
          player_navmesh->insert( obstacle_entt, obstacle_pos_cmp );

        auto dig_particle_uuid = Cmp::UUID::generate();
        Factory::Particle::add_obstacledig_ps( reg(), "graveyard.obstacle.dig.particle", 50, 2.f, 50.f, 14.f, dig_particle_uuid,
                                               obstacle_pos_cmp.position, obstacle_pos_cmp.y() );
      }
      else
      {
        // select all pickaxe sounds except the final smash sound
        Cmp::RandomInt random_picker( 1, 6 );
        m_sound_bank.get_effect( "pickaxe" + std::to_string( random_picker.gen() ) ).play();

        constexpr int kCracksPerHit = 3;
        for ( auto crack_iter : std::views::iota( 0, kCracksPerHit ) )
        {

          auto dig_particle_uuid = Cmp::UUID::generate();
          Factory::Particle::add_obstacledig_ps( reg(), "graveyard.obstacle.dig.particle", 5, 2.f, 50.f, 7.f, dig_particle_uuid,
                                                 obstacle_pos_cmp.position, obstacle_pos_cmp.y() );

          sf::Vector2f crack_start_pos( obstacle_pos_cmp.getCenter().x, obstacle_pos_cmp.getCenter().y - 12.f ); // offset for obstacle cap
          constexpr float kCrackLengthRatio = 0.4f;
          float crack_length = Constants::kGridSizePxF.x * kCrackLengthRatio;

          // Cracks land on one of the fixed kCrackAngleStepDeg-spaced slots around the circle (picked
          // at random, not in sequence), and never within kCrackAngleStepDeg of a crack already on
          // this obstacle (checked directly via circular distance, not slot indices, since the slot
          // grid itself is rotated per crack_iter below). The grid's own rotation is derived from the
          // obstacle's UUID so it's stable across hits but differs per obstacle instance, with each of
          // the kCracksPerHit cracks generated for this hit additionally offset from one another so
          // they don't all draw from the same rotated grid.
          constexpr float kCrackAngleStepDeg = 70.f;
          constexpr int kCrackSlotCount = static_cast<int>( 360.f / kCrackAngleStepDeg );
          float base_angle_deg = static_cast<float>( std::hash<Cmp::UUID>{}( obstacle_uuid_cmp ) % 360 ) +
                                 ( static_cast<float>( crack_iter ) * ( 360.f / static_cast<float>( kCracksPerHit ) ) );

          auto circular_angle_diff_deg = []( float a, float b )
          {
            float diff = std::fmod( std::abs( a - b ), 360.f );
            return diff > 180.f ? 360.f - diff : diff;
          };

          std::vector<float> used_angles_deg;
          for ( auto [ob_crack_entt, ob_crack_cmp, ob_crack_uuid] : reg().view<Cmp::ObstacleCrack, Cmp::UUID>().each() )
          {
            if ( ob_crack_uuid != obstacle_uuid_cmp ) continue;
            if ( ob_crack_cmp.sequence.size() < 2 or ob_crack_cmp.sequence.front().empty() or ob_crack_cmp.sequence.back().empty() ) continue;

            sf::Vector2f existing_dir = ob_crack_cmp.sequence.back().front().position - ob_crack_cmp.sequence.front().front().position;
            float existing_angle_deg = std::atan2( existing_dir.y, existing_dir.x ) * 180.f / std::numbers::pi_v<float>;
            if ( existing_angle_deg < 0.f ) existing_angle_deg += 360.f;
            used_angles_deg.push_back( existing_angle_deg );
          }

          std::vector<int> free_slots;
          for ( int slot = 0; slot < kCrackSlotCount; ++slot )
          {
            float candidate_deg = std::fmod( base_angle_deg + ( static_cast<float>( slot ) * kCrackAngleStepDeg ), 360.f );
            bool too_close = std::ranges::any_of( used_angles_deg, [&]( float used_deg )
                                                  { return circular_angle_diff_deg( candidate_deg, used_deg ) < kCrackAngleStepDeg; } );
            if ( not too_close ) free_slots.push_back( slot );
          }
          // every slot on this obstacle is already taken - fall back to a plain random pick
          int crack_slot = free_slots.empty() ? Cmp::RandomInt( 0, kCrackSlotCount - 1 ).gen()
                                              : free_slots[Cmp::RandomInt( 0, static_cast<int>( free_slots.size() ) - 1 ).gen()];

          float crack_angle_deg = std::fmod( base_angle_deg + ( static_cast<float>( crack_slot ) * kCrackAngleStepDeg ), 360.f );
          float crack_angle = crack_angle_deg * std::numbers::pi_v<float> / 180.f;
          sf::Vector2f crack_end_pos = crack_start_pos + sf::Vector2f{ std::cos( crack_angle ), std::sin( crack_angle ) } * crack_length;
          Cmp::ObstacleCrack::AngleDeviations crack_angles{ .inner = 1.f, .outer = 1.f };
          Cmp::ObstacleCrack obstacle_crack_cmp( crack_start_pos, crack_end_pos, crack_angles, sf::Time::Zero );
          for ( auto _ : std::views::iota( 0, 3 ) )
          {
            LightningSystem::divide_lightning_segments( obstacle_crack_cmp.sequence, obstacle_crack_cmp.m_deviations, 1 );
          }
          auto entt_main = reg().create();
          reg().emplace_or_replace<Cmp::ObstacleCrack>( entt_main, obstacle_crack_cmp );
          reg().emplace_or_replace<Cmp::UUID>( entt_main, obstacle_uuid_cmp );
        }
      }
    }
  }
}

void ActionSystem::check_player_dig_plant_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  // if ( inventory_slot_type != "sprite.item.shovel" and inventory_slot_type != "sprite.item.axe" and inventory_slot_type != "" ) { return; }

  // abort if still in cooldown
  if ( is_digging_on_cooldown() ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  reset_all_selected_positions();

  auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );

  auto position_view = reg().view<Cmp::PlantMultiBlock, Cmp::UUID>( entt::exclude<Cmp::SelectedPosition> );
  for ( auto [plant_entt, plant_mb_cmp, plant_uuid_cmp] : position_view.each() )
  {
    if ( mouse_position_bounds.findIntersection( plant_mb_cmp ) )
    {
      // check player is near obstacle that was mouse-selected
      if ( not Utils::Player::is_player_near( reg(), plant_mb_cmp ) ) continue;

      // check player is facing the obstacle
      if ( not Utils::Player::get_projected_position( reg() ).findIntersection( plant_mb_cmp ) ) continue;

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( plant_entt, plant_mb_cmp.position );

      reg().emplace_or_replace<Cmp::Player::DiggingCooldown>( Utils::Player::get_entity( reg() ) );

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      auto inventory_wear_view = reg().view<Cmp::PlayerInventorySlot>();
      if ( inventory_wear_view->empty() )
      {
        auto *plant_item = reg().try_get<Cmp::WorldItem>( plant_entt );
        if ( not plant_item ) continue;
        auto plantleaves_particle_uuid = Cmp::UUID::generate();
        Factory::Particle::add_plantleaves_ps( reg(), "graveyard.plant.leaves.particle", 50, 2.f, 50.f, 14.f, plantleaves_particle_uuid,
                                               plant_mb_cmp.getCenter(), plant_mb_cmp.position.y );
        Factory::Player::add_inventory( reg(), plant_item->item_type + ".drop" );
        Utils::Player::apply_action_from_inventory_item<Cmp::SpawnAction>( reg() );
        m_sound_bank.get_effect( "chopping_final" ).play();
      }
      else
      {
        if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }
        m_sound_bank.get_effect( "chopping_final" ).play();
        for ( auto [inventory_entt, inventory_slot] : inventory_wear_view.each() )
        {
          if ( inventory_slot.m_item.sprite_type == "sprite.item.shovel" )
          {
            auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
            auto player_pos = Utils::Player::get_position( reg() ).position;
            drop_inventory_item( player_pos, inventory_entt );
            Utils::Player::apply_action_from_world_item<Cmp::SpawnAction>( reg(), plant_entt );
            pickup_world_item( reg(), plant_entt );
          }
          else if ( inventory_slot.m_item.sprite_type == "sprite.item.axe" )
          {
            auto plantleaves_particle_uuid = Cmp::UUID::generate();
            Factory::Particle::add_plantleaves_ps( reg(), "graveyard.plant.leaves.particle", 50, 2.f, 50.f, 14.f, plantleaves_particle_uuid,
                                                   plant_mb_cmp.getCenter(), plant_mb_cmp.position.y );
            auto planttwigs_particle_uuid = Cmp::UUID::generate();
            Factory::Particle::add_planttwigs_ps( reg(), "graveyard.plant.twigs.particle", 10, 2.f, 50.f, 14.f, planttwigs_particle_uuid,
                                                  plant_mb_cmp.getCenter(), plant_mb_cmp.position.y );
            Utils::Player::apply_action_from_world_item<Cmp::DestroyAction>( reg(), plant_entt );
            Factory::Plant::remove_plant_mb( reg(), plant_entt, m_npc_navmesh.lock(), m_player_navmesh.lock() );
          }
        }

        get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG, plant_entt ) );
      }
    }
  }
}

void ActionSystem::drop_inventory_item( sf::Vector2f pos, entt::entity inventory_slot_entt )
{
  auto *inventory_slot_cmp = reg().try_get<Cmp::PlayerInventorySlot>( inventory_slot_entt );

  if ( not inventory_slot_cmp )
  {
    SPDLOG_INFO( "Player has no inventory" );
    return;
  }

  // if player inventory is a plant item then replant it in the ground - snap to nearest grid to prevent collision issues
  if ( inventory_slot_cmp->m_item.sprite_type.contains( "plant" ) )
  {
    // for plant drops fall through to the rest of the function that creates normal world items
    if ( not inventory_slot_cmp->m_item.sprite_type.contains( "drop" ) )
    {
      // multiblocks are top-left anchored, so offset the y-axis so that plant base is at players feet
      auto [mb_entt, segment_entt_list] = Factory::Multiblock::add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>(
          reg(), Utils::snap_to_grid( { pos.x, pos.y - Constants::kGridSizePxF.y } ),
          m_sprite_factory.get_spritesheet_by_type( inventory_slot_cmp->m_item.sprite_type ) );

      // Preserve the item this plant was grown from, so digging it back up (see the DIG handler in
      // on_player_action_event) can hand it back via the normal pickup_world_item path instead of
      // having to re-derive an item id from the multiblock's sprite.
      reg().emplace_or_replace<Cmp::WorldItem>( mb_entt, inventory_slot_cmp->m_item );

      // rebuild the m_player_navmesh here
      if ( auto player_navmesh = m_player_navmesh.lock() )
      {
        player_navmesh->clear();
        for ( auto [entt, nopath_cmp, pos_cmp] : reg().view<Cmp::Player::NoPath, Cmp::Position>().each() )
        {
          player_navmesh->insert( entt, pos_cmp );
        }
      }

      // The NPC navmesh is built once at scene setup, so evict the tiles the new plant
      // now blocks. Keep the player entity - PlayerSystem::update re-inserts it every
      // frame and NPCs need it to path towards the player.
      if ( auto npc_navmesh = m_npc_navmesh.lock() )
      {
        for ( auto seg_entt : segment_entt_list )
        {
          if ( not reg().any_of<Cmp::Npc::NoPathFinding>( seg_entt ) ) continue;
          auto seg_pos_cmp = reg().get<Cmp::Position>( seg_entt );
          for ( auto blocked_entt : npc_navmesh->at( seg_pos_cmp ) )
          {
            if ( reg().any_of<Cmp::Player::Character>( blocked_entt ) ) continue;
            npc_navmesh->remove( blocked_entt, seg_pos_cmp );
          }
        }
      }

      // clear player inevntory
      reg().destroy( inventory_slot_entt );
      m_sound_bank.get_effect( "chopping_final" ).play();
      return;
    }
  }

  // otherwise just drop it as a world item
  auto world_item_entt = reg().create();
  reg().emplace_or_replace<Cmp::Position>( world_item_entt, pos, Constants::kGridSizePxF );
  // clang-format off
  reg().emplace_or_replace<Cmp::AnimData>( world_item_entt, Cmp::AnimData::Config{  
        .sprite_type =  inventory_slot_cmp->m_item.sprite_type,
        .enabled = false
  });
  //clang-format on
  reg().emplace_or_replace<Cmp::ZOrderValue>( world_item_entt, pos.y - 1.f );
  reg().emplace_or_replace<Cmp::WorldItem>( world_item_entt, inventory_slot_cmp->m_item );
  reg().emplace_or_replace<Cmp::Npc::NoPathFinding>( world_item_entt );

  // try to copy any relevant components over to the new world carryitem entt
  auto *inventory_slot_level_cmp = reg().try_get<Cmp::Inventory::WearLevel>( inventory_slot_entt );
  if ( inventory_slot_level_cmp ) { reg().emplace_or_replace<Cmp::Inventory::WearLevel>( world_item_entt, inventory_slot_level_cmp->m_level ); }

  auto *inventory_scryingball_cmp = reg().try_get<Cmp::SeeingStone>( inventory_slot_entt );
  if ( inventory_scryingball_cmp ) { reg().emplace_or_replace<Cmp::SeeingStone>( world_item_entt, true, inventory_scryingball_cmp->target ); }

  auto *inventory_explosive_cmp = reg().try_get<Cmp::Explosive>( inventory_slot_entt );
  if ( inventory_explosive_cmp ) { reg().emplace_or_replace<Cmp::Explosive>( world_item_entt, false ); }

  auto *uuid_cmp = reg().try_get<Cmp::UUID>( inventory_slot_entt );
  if ( uuid_cmp )
  {

    for ( auto [ps_entt, ps_owner, ps_uuid_cmp] : reg().view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
    {
      if ( ps_uuid_cmp == *uuid_cmp )
      {
        ps_owner.sprite->set_view_type( Cmp::Particle::ViewType::WORLD );
        ps_owner.sprite->set_scale( Cmp::Particle::kWorldScalePreset );
        reg().emplace_or_replace<Cmp::ZOrderValue>( ps_entt, Utils::Player::get_position(reg()).y() - 1);
      }
    }
    reg().emplace_or_replace<Cmp::UUID>( world_item_entt, uuid_cmp->data );
  }

  // now destroy the inventory slot
  reg().destroy( inventory_slot_entt );
  if ( world_item_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
}

void ActionSystem::pickup_world_item( entt::registry &reg, entt::entity world_item_entt )
{
  
  auto *anim_data_cmp = reg.try_get<Cmp::AnimData>( world_item_entt );
  if ( not anim_data_cmp ) return;

  auto *world_item_cmp = reg.try_get<Cmp::WorldItem>(world_item_entt);
  if (not world_item_cmp) throw std::runtime_error("PlayerSystem::pickup_world_item - Unable to get world item component from " + std::to_string(static_cast<uint32_t>(world_item_entt)));

  // create the basic inventory slot entt
  auto inventory_entity = reg.create();
  reg.emplace_or_replace<Cmp::PlayerInventorySlot>( inventory_entity, *world_item_cmp );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( inventory_entity, Cmp::AnimData::Config{  
        .sprite_type =  world_item_cmp->sprite_type,
        .enabled = false
  });
  // clang-format on

  // transfer any component properties from the world item that we want to retain before it is destroyed
  auto *uuid_cmp = reg.try_get<Cmp::UUID>( world_item_entt );
  if ( uuid_cmp )
  {
    for ( auto [ps_entt, ps_owner, ps_uuid_cmp] : reg.view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
    {
      if ( ps_uuid_cmp == *uuid_cmp )
      {
        // Move the ParticleSprite to the UI view. Reset the scale, zorder and view type.
        ps_owner.sprite->clear();
        ps_owner.sprite->set_scale( Cmp::Particle::kUiScalePreset );
        ps_owner.sprite->set_view_type( Cmp::Particle::ViewType::SCREEN );
        reg.emplace_or_replace<Cmp::ZOrderValue>( ps_entt, 50000 );
      }
    }
    reg.emplace_or_replace<Cmp::UUID>( inventory_entity, uuid_cmp->data );
  }

  auto *wear_level_cmp = reg.try_get<Cmp::Inventory::WearLevel>( world_item_entt );
  if ( wear_level_cmp ) { reg.emplace_or_replace<Cmp::Inventory::WearLevel>( inventory_entity, wear_level_cmp->m_level ); }

  auto *scryingball_cmp = reg.try_get<Cmp::SeeingStone>( world_item_entt );
  if ( scryingball_cmp ) { reg.emplace_or_replace<Cmp::SeeingStone>( inventory_entity, false, scryingball_cmp->target ); }

  auto *explosive_cmp = reg.try_get<Cmp::Explosive>( world_item_entt );
  if ( explosive_cmp ) { reg.emplace_or_replace<Cmp::Explosive>( inventory_entity, false ); }

  // now destroy the world item entt
  Factory::Plant::remove_plant_mb( reg, world_item_entt, m_npc_navmesh.lock(), m_player_navmesh.lock() );
  if ( reg.valid( world_item_entt ) ) reg.destroy( world_item_entt );

  if ( inventory_entity != entt::null ) { m_sound_bank.get_effect( "get_loot" ).play(); }
}

} // namespace Game::Sys