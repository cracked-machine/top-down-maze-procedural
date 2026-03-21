#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerHealth.hpp>
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
  m_post_death_timer.reset();
}

void PlayerSystem::update( [[maybe_unused]] sf::Time globalDeltaTime, FootStepSfx footstep_sfx )
{

  // cache the player position so we can update the spatial grid afterwards.
  auto old_player_pos = Utils::Player::get_position( getReg() );

  // process changes to player position and related transforms
  localTransforms();

  if ( not m_post_death_timer.isRunning() )
  {
    update_player_position( globalDeltaTime, Utils::getSystemCmp( getReg() ).collisions_disabled );
    update_player_animation();
    update_player_zorder();

    // footstep sfx
    auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Direction>();
    for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
    {
      if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { stopFootstepsSound(); }
      else { playFootstepsSound( footstep_sfx ); }
    }
  }

  // did player die?
  check_player_mortality();

  if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
  {
    auto new_player_pos = Utils::Player::get_position( getReg() );
    pathfinding_navmesh->update( Utils::Player::get_entity( getReg() ), old_player_pos, new_player_pos );
  }
}

void PlayerSystem::localTransforms()
{

  auto blinking_player_view = getReg()
                                  .view<Cmp::PlayerCharacter, Cmp::Position, Cmp::Direction, Cmp::SpriteAnimation, Cmp::PlayerMortality,
                                        Cmp::AbsoluteAlpha, Cmp::AbsoluteRotation, Cmp::PlayerHealth>();
  for ( auto [entity, pc_cmp, pos_cmp, dir_cmp, anim_cmp, mortality_cmp, alpha_cmp, rotation_cmp, player_health_cmp] : blinking_player_view.each() )
  {
    // // normal do nothing
    // if ( mortality_cmp.state != Cmp::PlayerMortality::State::ALIVE ) continue;
    // wormhole jump fade effect
    auto *wormhole_jump = getReg().try_get<Cmp::WormholeJump>( entity );
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
      player_health_cmp.health = 0;
      mortality_cmp.state = Cmp::PlayerMortality::State::DEAD;
      return;
    }
    else
    {
      // damage cooldown blink effect
      auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( getReg() );
      bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
      int blink_visible = static_cast<int>( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;
      if ( !is_in_damage_cooldown || ( is_in_damage_cooldown && blink_visible ) ) { alpha_cmp = 255; }
      else { alpha_cmp = 0; }
    }
  }
}

void PlayerSystem::update_player_position( sf::Time globalDeltaTime, bool collision_disabled )
{

  Cmp::Position &player_pos = Utils::Player::get_position( getReg() );

  const Cmp::Direction raw_direction = Utils::Player::get_direction( getReg() );
  if ( raw_direction == sf::Vector2f( 0.f, 0.f ) ) return; // optimization

  auto &player_movement_speed = Sys::PersistSystem::get<Cmp::Persist::PlayerMovementSpeed>( getReg() );
  const float step = player_movement_speed.get_value() * globalDeltaTime.asSeconds();
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
  const Cmp::Direction direction_cmp = Utils::Player::get_direction( getReg() );
  Cmp::SpriteAnimation &anim_cmp = Utils::Player::get_sprite_anim( getReg() );

  // update the animation state based on movement direction
  if ( direction_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { anim_cmp.m_animation_active = false; }
  else
  {
    anim_cmp.m_animation_active = true;
    if ( direction_cmp.x == 1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.east"; }
    else if ( direction_cmp.x == -1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.west"; }
    else if ( direction_cmp.y == -1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.north"; }
    else if ( direction_cmp.y == 1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.south"; }
  }
}

void PlayerSystem::update_player_zorder()
{
  Cmp::ZOrderValue &zorder_cmp = Utils::Player::get_zorder( getReg() );
  const Cmp::Position player_pos = Utils::Player::get_position( getReg() );
  zorder_cmp.setZOrder( player_pos.position.y );
}

void PlayerSystem::on_player_mortality_event( ProceduralMaze::Events::PlayerMortalityEvent ev )
{

  auto common_death_throes = [&]()
  {
    m_post_death_timer.restart();
    getReg().remove<Cmp::SpriteAnimation>( Utils::Player::get_entity( getReg() ) );
    stopFootstepsSound();
    Utils::Player::get_health( getReg() ).health = 0;
    Utils::Player::get_mortality( getReg() ).state = Cmp::PlayerMortality::State::DEAD;
    SPDLOG_INFO( "Player is dead" );
  };

  switch ( ev.m_new_state )
  {
    case Cmp::PlayerMortality::State::ALIVE:
      break;

    case Cmp::PlayerMortality::State::FALLING: {
      SPDLOG_INFO( "Player is falling" );
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DECAYING: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::HAUNTED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::EXPLODING: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DROWNING: {
      break;
    }
    case Cmp::PlayerMortality::State::SQUISHED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SUICIDE: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::IGNITED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.lavaflames" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SKEWERED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SHOCKED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DEAD: {
      break;
    }
    case Cmp::PlayerMortality::State::SHADOWCURSED:
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::create_player_death_anim( getReg(), ev.m_death_pos, sprite );
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

    auto player_pos = Utils::Player::get_position( getReg() );
    Sprites::SpriteMetaType existing_player_inventory_type = "";

    // drop inventory if we have one
    auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
    for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
    {
      existing_player_inventory_type = inventory_cmp.type;
      auto dropped_entt = Factory::drop_inventory_slot_into_world( getReg(), player_pos,
                                                                   m_sprite_factory.get_multisprite_by_type( inventory_cmp.type ), inventory_entt );
      if ( dropped_entt != entt::null )
      {
        if ( existing_player_inventory_type.contains( "plant" ) ) { m_sound_bank.get_effect( "digging_earth" ).play(); }
        else { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
    }

    // pickup inventory if there is something at this position
    auto world_carryitem_view = getReg().view<Cmp::CarryItem, Cmp::Position>();
    for ( auto [carryitem_entt, carryitem_cmp, pos_cmp] : world_carryitem_view.each() )
    {
      if ( not player_pos.findIntersection( pos_cmp ) ) continue;           // is there something to pick up?
      if ( carryitem_cmp.type == existing_player_inventory_type ) continue; // dont pick up the one we just dropped
      if ( inventory_view.size() > 0 ) { break; }                           // don't pickup another if we already have one

      // ok pick it up
      if ( Factory::pickup_carry_item( getReg(), carryitem_entt ) != entt::null ) { m_sound_bank.get_effect( "get_loot" ).play(); }
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
  Cmp::RectBounds search_bounds( target_position.position, target_position.size, 1 );
  using namespace Utils::Collision;

  auto is_active = []( const Cmp::PlayerNoPath &playernopath ) { return playernopath.active; };
  if ( check_cmp<Cmp::PlayerNoPath>( getReg(), search_bounds, is_active ) ) { return false; }

  return true;
}

void PlayerSystem::check_player_mortality()
{

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position>();
  for ( auto [entity, pc_cmp, health_cmp, mortality_cmp, player_pos_cmp] : player_view.each() )
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
  for ( auto [player_entt, player_cmp] : getReg().view<Cmp::PlayerCharacter>().each() )
  {
    player_cmp.m_damage_cooldown_timer.stop();
  }
}

void PlayerSystem::enable_damage_cooldown()
{
  for ( auto [player_entt, player_cmp] : getReg().view<Cmp::PlayerCharacter>().each() )
  {
    player_cmp.m_damage_cooldown_timer.restart();
  }
}

void PlayerSystem::check_player_axe_npc_kill()
{
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock();
  if ( not pathfinding_navmesh ) return;

  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
  if ( inventory_slot_type != "CARRYITEM.axe" ) { return; }

  if ( Utils::Player::get_inventory_wear_level( getReg() ) <= 0 ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = getReg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    getReg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = getReg().view<Cmp::Position, Cmp::NPC, Cmp::SpriteAnimation>( entt::exclude<Cmp::SelectedPosition> );
  SPDLOG_DEBUG( "position_view size: {}", position_view.size_hint() );
  for ( auto [npc_entity, npc_pos_cmp, npc_cmp, anim_cmp] : position_view.each() )
  {
    if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) ) continue;
    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::getGameView() );
    if ( mouse_position_bounds.findIntersection( npc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found NPC entity at position: [{}, {}]!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
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
      getReg().emplace_or_replace<Cmp::SelectedPosition>( npc_entity, npc_pos_cmp.position );

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( getReg(), reduction_amount );

      // select the final smash sound
      m_sound_bank.get_effect( "axe_whip" ).play();
      m_sound_bank.get_effect( "skele_death" ).play();

      auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
      if ( inventory_slot_type == "CARRYITEM.axe" )
      {
        // drop loot - 1 in 3 chance
        auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
            std::vector<std::string>{ "EXTRA_HEALTH", "CHAIN_BOMBS", "WEAPON_BOOST" } );

        Cmp::RandomInt do_drop( 0, 2 );
        if ( do_drop.gen() == 0 )
        {
          auto dropped_loot_entt = Factory::create_loot_drop( getReg(), Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),
                                                              Cmp::RectBounds( npc_pos_cmp.position, npc_pos_cmp.size, 2.f ).getBounds(),
                                                              Factory::IncludePack<>{},
                                                              Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{},
                                                              Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{} );

          if ( dropped_loot_entt != entt::null )
          {
            auto player_pos = Utils::Player::get_position( getReg() );
            SPDLOG_INFO( "Player position was at {},{} when loot was dropped", player_pos.position.x, player_pos.position.y );
            m_sound_bank.get_effect( "drop_loot" ).play();
          }
        }

        // now destroy the NPC
        if ( getReg().valid( npc_entity ) )
        {
          pathfinding_navmesh->remove( npc_entity, npc_pos_cmp );
          getReg().destroy( npc_entity );
        }
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
    }
  }
}

} // namespace ProceduralMaze::Sys