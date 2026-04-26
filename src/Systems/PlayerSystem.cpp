#include <Constants.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Inventory/Explosive.hpp>
#include <Inventory/InventoryWearLevel.hpp>
#include <Inventory/ScryingBall.hpp>
#include <Stats/BaseAction.hpp>
#include <Stats/CarryAction.hpp>
#include <Stats/CollisionAction.hpp>
#include <Stats/ExhumeAction.hpp>
#include <Stats/PlayerStats.hpp>
#include <Stats/ProjectileAction.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Persistent/PlayerMovementSpeed.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

PlayerSystem::PlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                            entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
  std::ignore = get_systems_event_queue().sink<Events::PlayerMortalityEvent>().connect<&PlayerSystem::on_player_mortality_event>( this );
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&PlayerSystem::on_player_action_event>( this );
  std::ignore = get_systems_event_queue().sink<Events::DropInventoryEvent>().connect<&PlayerSystem::on_drop_inventory_event>( this );
  m_post_death_timer.reset();
}

void PlayerSystem::update( sf::Time dt, FootStepSfx footstep_sfx )
{

  // cache the player position so we can update the spatial grid afterwards.
  auto old_player_pos = Utils::Player::get_position( reg() );

  // process changes to player position and related transforms
  localTransforms();

  if ( not m_post_death_timer.isRunning() )
  {
    update_player_position( dt, Utils::getSystemCmp( reg() ).collisions_disabled );
    update_player_animation();
    update_player_zorder();

    // footstep sfx
    auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Direction>();
    for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
    {
      if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { stopFootstepsSound(); }
      else { playFootstepsSound( footstep_sfx ); }
    }
  }

  // did player die?
  check_player_mortality();

  check_timed_action_side_effects( dt );

  if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
  {
    auto new_player_pos = Utils::Player::get_position( reg() );
    pathfinding_navmesh->update( Utils::Player::get_entity( reg() ), old_player_pos, new_player_pos );
  }
}

void PlayerSystem::localTransforms()
{

  auto blinking_player_view = reg()
                                  .view<Cmp::PlayerCharacter, Cmp::Position, Cmp::Direction, Cmp::SpriteAnimation, Cmp::PlayerMortality,
                                        Cmp::AbsoluteAlpha, Cmp::AbsoluteRotation, Cmp::PlayerStats>();
  for ( auto [entity, pc_cmp, pos_cmp, dir_cmp, anim_cmp, mortality_cmp, alpha_cmp, rotation_cmp, player_stats_cmp] : blinking_player_view.each() )
  {
    // // normal do nothing
    // if ( mortality_cmp.state != Cmp::PlayerMortality::State::ALIVE ) continue;
    // wormhole jump fade effect
    auto *wormhole_jump = reg().try_get<Cmp::WormholeJump>( entity );
    if ( wormhole_jump )
    {
      // Calculate fade based on elapsed time vs total cooldown
      float elapsed = wormhole_jump->jump_clock.getElapsedTime().asSeconds();
      float cooldown = wormhole_jump->jump_cooldown.asSeconds();
      float progress = std::min( elapsed / cooldown, 1.0f ); // 0.0 to 1.0
      alpha_cmp = static_cast<uint8_t>( 255 * ( 1.0f - progress ) );
    }
    else if ( mortality_cmp.state == Cmp::PlayerMortality::State::FALLING )
    {
      // TODO: falling effect
      player_stats_cmp.apply_modifiers( { Cmp::Stats::Health{ -100 }, {}, {}, {}, {} } );
      mortality_cmp.state = Cmp::PlayerMortality::State::DEAD;
      return;
    }
    else
    {
      // damage cooldown blink effect
      auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( reg() );
      bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
      bool blink_visible = ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;
      if ( not is_in_damage_cooldown || ( is_in_damage_cooldown && blink_visible ) ) { alpha_cmp = 255; }
      else { alpha_cmp = 0; }
    }
  }
}

void PlayerSystem::update_player_position( sf::Time dt, bool collision_disabled )
{

  Cmp::Position &player_pos = Utils::Player::get_position( reg() );

  const Cmp::Direction raw_direction = Utils::Player::get_direction( reg() );
  if ( raw_direction == sf::Vector2f( 0.f, 0.f ) ) return; // optimization

  auto &player_movement_speed = Sys::PersistSystem::get<Cmp::Persist::PlayerMovementSpeed>( reg() );
  const float step = player_movement_speed.get_value() * dt.asSeconds();
  const Cmp::Direction direction = raw_direction.componentWiseMul( { step, step } );

  const sf::FloatRect next_horizontal_move( { player_pos.position.x + direction.x, player_pos.position.y }, player_pos.size );
  const sf::FloatRect next_vertical_move( { player_pos.position.x, player_pos.position.y + direction.y }, player_pos.size );

  bool can_move = false;
  bool moved_perp = false;
  Cmp::Direction resolved_dir_vector;

  if ( is_valid_move( next_horizontal_move ) or collision_disabled )
  {
    can_move = true;
    resolved_dir_vector.x = direction.x;
  }
  else if ( direction.x != 0.0f )
  {

    const float y_offset = Utils::snap_to_grid( player_pos ).position.y - player_pos.position.y;
    const float nudge = std::copysign( std::min( std::abs( y_offset ), step ), y_offset );
    const sf::FloatRect nudged( { player_pos.position.x + direction.x, player_pos.position.y + nudge }, player_pos.size );
    if ( is_valid_move( nudged ) and not moved_perp )
    {
      moved_perp = true;
      can_move = true;
      resolved_dir_vector.x = direction.x;
      resolved_dir_vector.y = nudge;
    }
    else
    {
      // ensure player moves fully against obstacle edge
      player_pos.position.x = Utils::snap_to_grid( player_pos ).position.x;
    }
  }

  if ( is_valid_move( next_vertical_move ) or collision_disabled )
  {
    can_move = true;
    resolved_dir_vector.y = direction.y;
  }
  else if ( direction.y != 0.0f )
  {
    const float x_offset = Utils::snap_to_grid( player_pos ).position.x - player_pos.position.x;
    const float nudge = std::copysign( std::min( std::abs( x_offset ), step ), x_offset );
    const sf::FloatRect nudged( { player_pos.position.x + nudge, player_pos.position.y + direction.y }, player_pos.size );
    if ( is_valid_move( nudged ) and not moved_perp )
    {
      moved_perp = true;
      can_move = true;
      resolved_dir_vector.x = nudge;
      resolved_dir_vector.y = direction.y;
    }
    else
    {
      // else ensure player moves fully against obstacle edge
      player_pos.position.y = Utils::snap_to_grid( player_pos ).position.y;
    }
  }

  if ( can_move ) { player_pos.position += resolved_dir_vector; }
}

void PlayerSystem::update_player_animation()
{
  const Cmp::Direction direction_cmp = Utils::Player::get_direction( reg() );
  Cmp::SpriteAnimation &anim_cmp = Utils::Player::get_sprite_anim( reg() );

  // update the animation state based on movement direction
  if ( direction_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { anim_cmp.m_animation_active = false; }
  else
  {
    anim_cmp.m_animation_active = true;
    if ( direction_cmp.x == 1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.east"; }
    else if ( direction_cmp.x == -1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.west"; }
    else if ( direction_cmp.y == -1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.north"; }
    else if ( direction_cmp.y == 1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.south"; }
  }
}

void PlayerSystem::update_player_zorder()
{
  Cmp::ZOrderValue &zorder_cmp = Utils::Player::get_zorder( reg() );
  const Cmp::Position player_pos = Utils::Player::get_position( reg() );
  zorder_cmp.setZOrder( player_pos.position.y );
}

void PlayerSystem::on_player_mortality_event( ProceduralMaze::Events::PlayerMortalityEvent ev )
{

  auto common_death_throes = [&]()
  {
    m_post_death_timer.restart();
    reg().remove<Cmp::SpriteAnimation>( Utils::Player::get_entity( reg() ) );
    stopFootstepsSound();
    Utils::Player::get_player_stats( reg() ).apply_modifiers( { Cmp::Stats::Health{ -100 }, {}, {}, {}, {} } );
    Utils::Player::get_mortality( reg() ).state = Cmp::PlayerMortality::State::DEAD;
    SPDLOG_INFO( "Player is dead" );
  };

  switch ( ev.m_new_state )
  {
    case Cmp::PlayerMortality::State::ALIVE:
      break;

    case Cmp::PlayerMortality::State::FALLING: {
      SPDLOG_INFO( "Player is falling" );
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DECAYING: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::HAUNTED: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::EXPLODING: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DROWNING: {
      break;
    }
    case Cmp::PlayerMortality::State::SQUISHED: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SUICIDE: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::IGNITED: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.lavaflames" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SKEWERED: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SHOCKED: {
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DEAD: {
      break;
    }
    case Cmp::PlayerMortality::State::SHADOWCURSED:
      const auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
  }
}

void PlayerSystem::on_player_action_event( ProceduralMaze::Events::PlayerActionEvent ev )
{

  if ( ev.action == Events::PlayerActionEvent::GameActions::DROP_CARRYITEM )
  {
    if ( m_inventory_cooldown_timer.getElapsedTime() < sf::milliseconds( 750.f ) ) return;

    auto player_pos = Utils::Player::get_position( reg() );
    Sprites::SpriteMetaType existing_player_inventory_type;

    // drop inventory if we have one
    auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
    for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
    {
      existing_player_inventory_type = inventory_cmp.m_item.sprite_type;
      auto dropped_entt = drop_inventory_slot_into_world( player_pos.position, inventory_entt );
      if ( dropped_entt != entt::null )
      {
        if ( existing_player_inventory_type.contains( "plant" ) ) { m_sound_bank.get_effect( "digging_earth" ).play(); }
        else { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
    }

    // pickup inventory if there is something at this position
    auto world_carryitem_view = reg().view<Cmp::InventoryItem, Cmp::Position>();
    for ( auto [carryitem_entt, carryitem_cmp, pos_cmp] : world_carryitem_view.each() )
    {
      if ( not player_pos.findIntersection( pos_cmp ) ) continue;                  // is there something to pick up?
      if ( carryitem_cmp.sprite_type == existing_player_inventory_type ) continue; // dont pick up the one we just dropped
      if ( inventory_view.size() > 0 ) { break; }                                  // don't pickup another if we already have one

      // ok pick it up
      if ( Factory::pickup_world_item( reg(), carryitem_entt ) != entt::null ) { m_sound_bank.get_effect( "get_loot" ).play(); }
    }
    m_inventory_cooldown_timer.restart();
    SPDLOG_DEBUG( "inventory_view: {} ", inventory_view.size() );
  }
  else if ( ev.action == ProceduralMaze::Events::PlayerActionEvent::GameActions::ATTACK )
  {
    // axe attack?!
    check_player_axe_npc_kill();
  }
}

bool PlayerSystem::is_valid_move( const sf::FloatRect &target_position )
{
  auto search_bounds = Cmp::RectBounds::scaled( target_position.position, target_position.size, 1 );
  using namespace Utils::Collision;

  auto is_active = []( const Cmp::PlayerNoPath &playernopath ) { return playernopath.active; };
  return not check_cmp<Cmp::PlayerNoPath>( reg(), search_bounds, is_active );
}

void PlayerSystem::check_player_mortality()
{

  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::PlayerMortality, Cmp::Position>();
  for ( auto [entity, pc_cmp, mortality_cmp, player_pos_cmp] : player_view.each() )
  {
    if ( ( mortality_cmp.state == Cmp::PlayerMortality::State::DEAD ) and ( m_post_death_timer.getElapsedTime() > sf::seconds( 5.f ) ) )
    {

      SPDLOG_DEBUG( "Player has progressed to deadness." );
      m_post_death_timer.reset();
      stopFootstepsSound();

      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::GAME_OVER );
    }
  }
}

void PlayerSystem::check_timed_action_side_effects( sf::Time dt )
{
  Cmp::PlayerStats &stats_cmp = Utils::Player::get_player_stats( reg() );
  for ( auto [slot_entt, slot_cmp] : reg().view<Cmp::PlayerInventorySlot>().each() )
  {
    for ( auto &[action_type, item_action_pair] : slot_cmp.m_item.actions )
    {
      if ( action_type == std::type_index( typeid( Cmp::CollisionAction ) ) ) continue;
      if ( action_type == std::type_index( typeid( Cmp::ProjectileAction ) ) ) continue;
      auto &[item_action, item_action_timer] = item_action_pair;
      if ( item_action.interval() == 0.f ) continue;
      item_action_timer += dt;
      if ( item_action_timer.asSeconds() < item_action.interval() ) continue;
      stats_cmp.apply_modifiers( item_action );
      item_action_timer = sf::Time::Zero;
    }
  }
  for ( auto [npc_entt, npc_cmp] : reg().view<Cmp::NPC>().each() )
  {
    for ( auto &[action_type, npc_action_pair] : npc_cmp.actions )
    {
      if ( action_type == std::type_index( typeid( Cmp::CollisionAction ) ) ) continue;
      if ( action_type == std::type_index( typeid( Cmp::ProjectileAction ) ) ) continue;
      auto &[npc_action, npc_action_timer] = npc_action_pair;
      if ( npc_action.interval() == 0.f ) continue;
      npc_action_timer += dt;
      if ( npc_action_timer.asSeconds() < npc_action.interval() ) continue;
      stats_cmp.apply_modifiers( npc_action );
      npc_action_timer = sf::Time::Zero;
    }
  }
}

entt::entity PlayerSystem::drop_inventory_slot_into_world( sf::Vector2f pos, entt::entity inventory_slot_entt )
{
  auto *inventory_slot_cmp = reg().try_get<Cmp::PlayerInventorySlot>( inventory_slot_entt );

  if ( not inventory_slot_cmp )
  {
    SPDLOG_INFO( "Player has no inventory" );
    return entt::null;
  }

  // if plant then replant it in the ground - snap to nearest grid to prevent collision issues
  if ( inventory_slot_cmp->m_item.sprite_type.contains( "plant" ) )
  {
    auto world_item_entt = Factory::create_plant_obstacle( reg(), Cmp::Position( Utils::snap_to_grid( pos ), Constants::kGridSizePxF ),
                                                           m_sprite_factory.get_multisprite_by_type( inventory_slot_cmp->m_item.sprite_type ) );
    reg().destroy( inventory_slot_entt );
    return world_item_entt;
  }

  // otherwise just drop it as a Re-pickupable item
  auto world_item_entt = reg().create();
  reg().emplace_or_replace<Cmp::Position>( world_item_entt, pos, Constants::kGridSizePxF );
  reg().emplace_or_replace<Cmp::SpriteAnimation>( world_item_entt, 0, 0, false, inventory_slot_cmp->m_item.sprite_type, 0 );
  reg().emplace_or_replace<Cmp::ZOrderValue>( world_item_entt, pos.y - 1.f );
  reg().emplace_or_replace<Cmp::InventoryItem>( world_item_entt, inventory_slot_cmp->m_item );
  reg().emplace_or_replace<Cmp::NpcNoPathFinding>( world_item_entt );

  // try to copy any relevant components over to the new world carryitem entt
  auto *inventory_slot_level_cmp = reg().try_get<Cmp::InventoryWearLevel>( inventory_slot_entt );
  if ( inventory_slot_level_cmp ) { reg().emplace_or_replace<Cmp::InventoryWearLevel>( world_item_entt, inventory_slot_level_cmp->m_level ); }

  auto *inventory_scryingball_cmp = reg().try_get<Cmp::ScryingBall>( inventory_slot_entt );
  if ( inventory_scryingball_cmp ) { reg().emplace_or_replace<Cmp::ScryingBall>( world_item_entt, true, inventory_scryingball_cmp->target ); }

  auto *inventory_explosive_cmp = reg().try_get<Cmp::Explosive>( inventory_slot_entt );
  if ( inventory_explosive_cmp ) { reg().emplace_or_replace<Cmp::Explosive>( world_item_entt, false ); }

  // now destroy the inventory slot
  reg().destroy( inventory_slot_entt );
  return world_item_entt;
}

void PlayerSystem::on_drop_inventory_event( ProceduralMaze::Events::DropInventoryEvent ev )
{
  auto dropped_entt = drop_inventory_slot_into_world( ev.drop_pos, ev.inventory_slot_entt );
  if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
}

void PlayerSystem::playFootstepsSound( FootStepSfx type )
{
  switch ( type )
  {
    case FootStepSfx::NONE:
      break;
    case FootStepSfx::GRAVEL: {
      // Restarting prematurely creates a stutter effect, so check first
      auto &footsteps = m_sound_bank.get_effect( "footsteps" );
      if ( footsteps.getStatus() == sf::Sound::Status::Playing ) return;
      footsteps.play();
      break;
    }
    case FootStepSfx::FLOORBOARDS: {
      // // Restarting prematurely creates a stutter effect, so check first
      // auto &footsteps = m_sound_bank.get_effect( "footsteps" );
      // if ( footsteps.getStatus() == sf::Sound::Status::Playing ) return;
      // footsteps.play();
      break;
    }
  }
}

void PlayerSystem::stopFootstepsSound()
{
  // add more footstep sfx here when needed
  m_sound_bank.get_effect( "footsteps" ).stop();
}

void PlayerSystem::disable_damage_cooldown()
{
  for ( auto [player_entt, player_cmp] : reg().view<Cmp::PlayerCharacter>().each() )
  {
    player_cmp.m_damage_cooldown_timer.stop();
  }
}

void PlayerSystem::enable_damage_cooldown()
{
  for ( auto [player_entt, player_cmp] : reg().view<Cmp::PlayerCharacter>().each() )
  {
    player_cmp.m_damage_cooldown_timer.restart();
  }
}

void PlayerSystem::check_player_axe_npc_kill()
{
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock();
  if ( not pathfinding_navmesh ) return;

  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( inventory_slot_type != "sprite.item.axe" ) { return; }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::NPC, Cmp::SpriteAnimation>( entt::exclude<Cmp::SelectedPosition> );
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
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
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
      m_sound_bank.get_effect( "skele_death" ).play();

      auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
      if ( inventory_slot_type == "sprite.item.axe" )
      {
        // drop loot - 1 in 3 chance
        auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
            std::vector<std::string>{ "sprite.graveyard.loot.health", "sprite.graveyard.loot.blast", "sprite.graveyard.loot.repair" } );

        Cmp::RandomInt do_drop( 0, 2 );
        if ( do_drop.gen() == 0 )
        {
          auto dropped_loot_entt = Factory::create_loot_drop( reg(), Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),
                                                              Cmp::RectBounds::scaled( npc_pos_cmp.position, npc_pos_cmp.size, 2.f ).getBounds(),
                                                              Factory::IncludePack<>{},
                                                              Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{},
                                                              Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{} );

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
          Factory::destroy_npc( reg(), npc_entity );
        }
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
    }
  }
}

} // namespace ProceduralMaze::Sys