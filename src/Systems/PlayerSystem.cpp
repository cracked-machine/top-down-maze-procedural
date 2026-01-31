
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/PlayerSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Direction.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerDetectionBounds.hpp>
#include <Components/Player/PlayerDistance.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

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

void PlayerSystem::on_player_mortality_event( ProceduralMaze::Events::PlayerMortalityEvent ev )
{

  // clang-format off
  auto common_death_throes = [&]()
  {
    m_post_death_timer.restart();
    getReg().remove<Cmp::SpriteAnimation>( Utils::get_player_entity( getReg() ) );
    stopFootstepsSound();
    Utils::get_player_health( getReg() ).health = 0;
    Utils::get_player_mortality( getReg() ).state = Cmp::PlayerMortality::State::DEAD;
  };
  // clang-format on

  switch ( ev.m_new_state )
  {
    case Cmp::PlayerMortality::State::ALIVE:
      break;

    case Cmp::PlayerMortality::State::FALLING: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DECAYING: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::HAUNTED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::EXPLODING: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DROWNING: {
      break;
    }
    case Cmp::PlayerMortality::State::SQUISHED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SUICIDE: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::IGNITED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.lavaflames" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SKEWERED: {
      auto &sprite = m_sprite_factory.get_multisprite_by_type( "PLAYERDEATH.bloodsplat" );
      Factory::createPlayerDeathAnim( getReg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DEAD: {
      break;
    }
  }
}

void PlayerSystem::on_player_action_event( ProceduralMaze::Events::PlayerActionEvent ev )
{

  if ( ev.action == Events::PlayerActionEvent::GameActions::DROP_CARRYITEM )
  {
    if ( m_inventory_cooldown_timer.getElapsedTime() < sf::milliseconds( 750.f ) ) return;

    auto player_pos = Utils::get_player_position( getReg() );
    Sprites::SpriteMetaType existing_player_inventory_type = "";

    // drop inventory if we have one
    auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
    for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
    {
      existing_player_inventory_type = inventory_cmp.type;
      auto dropped_entt = Factory::dropInventorySlotIntoWorld( getReg(), player_pos, m_sprite_factory.get_multisprite_by_type( inventory_cmp.type ),
                                                               inventory_entt );
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
      if ( Factory::pickupCarryItem( getReg(), carryitem_entt ) != entt::null ) { m_sound_bank.get_effect( "get_loot" ).play(); }
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

void PlayerSystem::update( sf::Time globalDeltaTime )
{

  // process changes to player position and related transforms
  localTransforms();

  if ( not m_post_death_timer.isRunning() )
  {

    // process global movement, disable collision detection if option set
    globalTranslations( globalDeltaTime, Utils::getSystemCmp( getReg() ).collisions_enabled );
    // footstep sfx
    auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Direction>();
    for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
    {
      if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { stopFootstepsSound(); }
      else { playFootstepsSound(); }
    }
    // update path tracking data
    if ( m_debug_info_timer.getElapsedTime() >= sf::milliseconds( 100 ) )
    {
      refreshPlayerDistances();
      m_debug_info_timer.restart();
    }

    if ( Utils::getSystemCmp( getReg() ).collisions_enabled )
    {
      // update player health if hit by shockwave
      for ( auto entt : getReg().view<Cmp::NpcShockwave>() )
      {
        checkShockwavePlayerCollision( getReg().get<Cmp::NpcShockwave>( entt ) );
      }
    }
  }

  // did player die?
  checkPlayerMortality();
}

void PlayerSystem::checkPlayerMortality()
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

void PlayerSystem::playFootstepsSound()
{
  // Restarting prematurely creates a stutter effect, so check first
  auto &footsteps = m_sound_bank.get_effect( "footsteps" );
  if ( footsteps.getStatus() == sf::Sound::Status::Playing ) return;
  footsteps.play();
}

void PlayerSystem::stopFootstepsSound() { m_sound_bank.get_effect( "footsteps" ).stop(); }

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
      auto &pc_damage_cooldown = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PcDamageDelay>( getReg() );
      bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
      int blink_visible = static_cast<int>( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;
      if ( !is_in_damage_cooldown || ( is_in_damage_cooldown && blink_visible ) ) { alpha_cmp = 255; }
      else { alpha_cmp = 0; }
    }
  }
}

void PlayerSystem::globalTranslations( sf::Time globalDeltaTime, bool collision_detection )
{

  const float dt = globalDeltaTime.asSeconds();

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds, Cmp::SpriteAnimation>();
  for ( auto [entity, pc_cmp, pos_cmp, dir_cmp, pc_detection_bounds, anim_cmp] : player_view.each() )
  {
    // always set the player, even if not moving
    auto zorder_cmp = getReg().try_get<Cmp::ZOrderValue>( entity );
    if ( zorder_cmp ) { zorder_cmp->setZOrder( pos_cmp.position.y ); }

    auto lerp_cmp = getReg().try_get<Cmp::LerpPosition>( entity );
    bool wants_to_move = dir_cmp != sf::Vector2f( 0.0f, 0.0f );

    // update the animation state based on movement direction
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) )
    {
      // player is not pressing any keys but sprite is still lerping to target position?
      // keep animation active otherwise it has the effect of sliding to a stop
      if ( lerp_cmp && lerp_cmp->m_lerp_factor < 1.0f ) { anim_cmp.m_animation_active = true; }
      else { anim_cmp.m_animation_active = false; }
    }
    else
    {
      anim_cmp.m_animation_active = true;
      if ( dir_cmp.x == 1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.east"; }
      else if ( dir_cmp.x == -1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.west"; }
      else if ( dir_cmp.y == -1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.north"; }
      else if ( dir_cmp.y == 1 ) { anim_cmp.m_sprite_type = "PLAYER.walk.south"; }
    }

    // Only start new movement when not lerping
    if ( wants_to_move && !lerp_cmp )
    {
      // disable new lerp starting during wormhole jump
      if ( getReg().try_get<Cmp::WormholeJump>( entity ) ) return;

      // make a copy to determine if new target position is valid
      sf::FloatRect new_pos{ pos_cmp };
      new_pos.position.x = pos_cmp.position.x + ( dir_cmp.x * Constants::kGridSquareSizePixels.x );
      new_pos.position.y = pos_cmp.position.y + ( dir_cmp.y * Constants::kGridSquareSizePixels.y );

      // Check collision ONCE
      bool can_move = not collision_detection || is_valid_move( new_pos );
      if ( !can_move ) continue; // Early exit if blocked

      // Check if moving diagonally AFTER we know movement is valid
      bool is_diagonal = ( dir_cmp.x != 0.0f ) && ( dir_cmp.y != 0.0f );
      bool diagonal_between_obstacles = is_diagonal && isDiagonalMovementBetweenObstacles( pos_cmp, dir_cmp );

      auto &player_lerp_speed = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerLerpSpeed>( getReg() );
      auto &diagonal_lerp_speed_modifier = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerDiagonalLerpSpeedModifier>( getReg() );
      auto &shortcut_lerp_speed_modifier = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerShortcutLerpSpeedModifier>( getReg() );

      float speed_modifier = 1.0f;
      if ( diagonal_between_obstacles )
      {
        // Check if shortcut movement is disabled (speed modifier at or near zero)
        if ( shortcut_lerp_speed_modifier.get_value() < 0.01f )
        {
          // Block this movement entirely instead of making it super slow
          continue; // Skip to next entity, don't start the movement
        }

        // Extra slow when squeezing between obstacles
        speed_modifier = shortcut_lerp_speed_modifier.get_value();
      }
      else if ( is_diagonal )
      {
        // Normal diagonal slowdown
        speed_modifier = diagonal_lerp_speed_modifier.get_value();
      }

      float adjusted_speed = player_lerp_speed.get_value() * speed_modifier;

      getReg().emplace<Cmp::LerpPosition>( entity, new_pos.position, adjusted_speed );
      lerp_cmp = getReg().try_get<Cmp::LerpPosition>( entity );

      lerp_cmp->m_start = pos_cmp.position;
      lerp_cmp->m_target = new_pos.position;
      lerp_cmp->m_lerp_factor = 0.0f;
    }

    // now we modify... ongoing lerp movement
    if ( lerp_cmp && lerp_cmp->m_lerp_factor < 1.0f )
    {
      lerp_cmp->m_lerp_factor += ( lerp_cmp->m_lerp_speed * dt );
      lerp_cmp->m_lerp_factor = std::min( lerp_cmp->m_lerp_factor, 1.0f );

      // interpolate to the new position
      const float t = lerp_cmp->m_lerp_factor;
      const float one_minus_t = 1.0f - t;

      // Simple manual lerp - 33 lines of assembly vs 134 for std::lerp vs 54 for std::fma
      // https://godbolt.org/z/YdeKco5d6
      pos_cmp.position.x = one_minus_t * lerp_cmp->m_start.x + t * lerp_cmp->m_target.x;
      pos_cmp.position.y = one_minus_t * lerp_cmp->m_start.y + t * lerp_cmp->m_target.y;

      // Update detection bounds position during lerp to keep in sync
      pc_detection_bounds.position( pos_cmp.position );

      // if lerp is complete, finalize position
      if ( lerp_cmp->m_lerp_factor >= 1.0f )
      {
        pos_cmp.position = lerp_cmp->m_target;
        pc_detection_bounds.position( pos_cmp.position );
        getReg().remove<Cmp::LerpPosition>( entity );
      }
    }
  }
}

void PlayerSystem::refreshPlayerDistances()
{
  const auto viewBounds = Utils::calculate_view_bounds( RenderSystem::getGameView() );

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_db_cmp] : player_view.each() )
  {
    auto add_path_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::NoPathFinding> );
    for ( auto [path_entt, path_pos_cmp] : add_path_view.each() )
    {
      if ( getReg().all_of<Cmp::FootStepTimer>( path_entt ) )
      {
        // always update footsteps distance to player
        if ( !Utils::is_visible_in_view( viewBounds, path_pos_cmp ) ) continue;
        auto distance = std::floor( Utils::Maths::getEuclideanDistance( pc_pos_cmp.position, path_pos_cmp.position ) );
        getReg().emplace_or_replace<Cmp::PlayerDistance>( path_entt, distance );
      }
      else
      {
        if ( pc_db_cmp.findIntersection( path_pos_cmp ) )
        {
          if ( !Utils::is_visible_in_view( viewBounds, path_pos_cmp ) ) continue; // optimization

          // calculate the distance from the position to the player
          auto distance = std::floor( Utils::Maths::getEuclideanDistance( pc_pos_cmp.position, path_pos_cmp.position ) );
          getReg().emplace_or_replace<Cmp::PlayerDistance>( path_entt, distance );
        }
      }
    }

    auto remove_path_view = getReg().view<Cmp::Position>();
    for ( auto [path_entt, path_pos_cmp] : remove_path_view.each() )
    {
      // keep playerdistance for footsteps always so NPC can track outside of the detection bounds
      if ( getReg().all_of<Cmp::FootStepTimer>( path_entt ) ) continue;
      // otherwise tidy up any out of range player distances
      if ( not pc_db_cmp.findIntersection( path_pos_cmp ) ) { getReg().remove<Cmp::PlayerDistance>( path_entt ); }
    }
  }
}

void PlayerSystem::checkShockwavePlayerCollision( Cmp::NpcShockwave &shockwave )
{
  auto &pc_damage_cooldown = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PcDamageDelay>( getReg() );
  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PlayerHealth>();

  for ( auto [player_entity, player_cmp, player_pos, player_health] : player_view.each() )
  {
    if ( player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() ) continue;
    if ( Sys::ShockwaveSystem::intersectsWithVisibleSegments( getReg(), shockwave, player_pos ) )
    {
      player_health.health -= 10;
      m_sound_bank.get_effect( "damage_player" ).play();
      player_cmp.m_damage_cooldown_timer.restart();
      SPDLOG_INFO( "Player (health:{}) INTERSECTS with Shockwave (position: {},{} - effective_radius: {})", player_health.health,
                   shockwave.sprite.getPosition().x, shockwave.sprite.getPosition().y, shockwave.sprite.getRadius() );
    }
    else { SPDLOG_DEBUG( "Player does NOT intersect with shockwave (distance: {}, effective_radius: {})", distance, effective_radius ); }
  }
}

void PlayerSystem::check_player_axe_npc_kill()
{
  auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
  if ( inventory_slot_type != "CARRYITEM.axe" ) { return; }

  if ( Utils::get_player_inventory_wear_level( getReg() ) <= 0 ) { return; }

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
      SPDLOG_DEBUG( "Found NPC entity at position: [{}, {}]!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
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

      float reduction_amount = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::WeaponDegradePerHit>( getReg() ).get_value();
      Utils::reduce_player_inventory_wear_level( getReg(), reduction_amount );

      // select the final smash sound
      m_sound_bank.get_effect( "axe_whip" ).play();
      m_sound_bank.get_effect( "skele_death" ).play();

      auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
      if ( inventory_slot_type == "CARRYITEM.axe" )
      {
        // drop loot - 1 in 3 chance
        auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
            std::vector<std::string>{ "EXTRA_HEALTH", "CHAIN_BOMBS", "WEAPON_BOOST" } );

        Cmp::RandomInt do_drop( 0, 2 );
        if ( do_drop.gen() == 0 )
        {
          // clang-format off
          auto dropped_loot_entt = Factory::createLootDrop( 
            getReg(), 
            Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),                                        
            Cmp::RectBounds(npc_pos_cmp.position, npc_pos_cmp.size, 2.f).getBounds(),
            Factory::IncludePack<>{},
            Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{},
            Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{} );
          // clang-format on

          if ( dropped_loot_entt != entt::null )
          {
            auto player_pos = Utils::get_player_position( getReg() );
            SPDLOG_INFO( "Player position was at {},{} when loot was dropped", player_pos.position.x, player_pos.position.y );
            m_sound_bank.get_effect( "drop_loot" ).play();
          }
        }

        // now destroy the NPC
        if ( getReg().valid( npc_entity ) ) getReg().destroy( npc_entity );
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
    }
  }
}

} // namespace ProceduralMaze::Sys