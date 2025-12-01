#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WeaponLevel.hpp>
#include <Components/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

#include <Events/SceneManagerEvent.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sys
{

PlayerSystem::PlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                            entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
}

void PlayerSystem::update( sf::Time globalDeltaTime, bool skip_collision_check )
{

  localTransforms();
  globalTranslations( globalDeltaTime, skip_collision_check );

  if ( m_debug_info_timer.getElapsedTime() >= sf::milliseconds( 100 ) )
  {
    refreshPlayerDistances();
    m_debug_info_timer.restart();
  }

  checkPlayerMortality();

  // play/stop footstep sounds depending on player movement
  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Direction>();
  for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { stopFootstepsSound(); }
    else { playFootstepsSound(); }
  }
}

void PlayerSystem::addPlayerEntity()
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = getReg().create();

  // start position must be pixel coordinates within the screen resolution (kDisplaySize),
  // but also grid aligned (kMapGridSize) to avoid collision detection errors.
  // So we must recalc start position to the nearest grid position here
  auto start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  start_pos = snap_to_grid( start_pos );
  getReg().emplace<Cmp::Position>( entity, start_pos, kGridSquareSizePixelsF );

  auto &bomb_inventory = get_persistent_component<Cmp::Persistent::BombInventory>();
  auto &blast_radius = get_persistent_component<Cmp::Persistent::BlastRadius>();
  getReg().emplace<Cmp::PlayableCharacter>( entity, bomb_inventory.get_value(), blast_radius.get_value() );

  getReg().emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();

  getReg().emplace<Cmp::PCDetectionBounds>( entity, start_pos, kGridSquareSizePixelsF, pc_detection_scale.get_value() );

  getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYER.walk.south" );
  getReg().emplace<Cmp::PlayerCandlesCount>( entity, 0 );
  getReg().emplace<Cmp::PlayerKeysCount>( entity, 0 );
  getReg().emplace<Cmp::PlayerRelicCount>( entity, 0 );
  getReg().emplace<Cmp::PlayerHealth>( entity, 100 );
  getReg().emplace<Cmp::PlayerMortality>( entity, Cmp::PlayerMortality::State::ALIVE );
  getReg().emplace<Cmp::WeaponLevel>( entity, 100.f );
  getReg().emplace<Cmp::ZOrderValue>( entity, start_pos.y ); // z-order based on y-position
  getReg().emplace<Cmp::AbsoluteAlpha>( entity, 255 );       // fully opaque
  getReg().emplace<Cmp::AbsoluteRotation>( entity, 0 );
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
                                  .view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::SpriteAnimation, Cmp::PlayerMortality,
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
    // damage cooldown blink effect
    else
    {
      auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
      bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
      int blink_visible = static_cast<int>( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;
      if ( !is_in_damage_cooldown || ( is_in_damage_cooldown && blink_visible ) ) { alpha_cmp = 255; }
      else { alpha_cmp = 0; }
    }
  }
}

void PlayerSystem::globalTranslations( sf::Time globalDeltaTime, bool skip_collision_check )
{

  const float dt = globalDeltaTime.asSeconds();

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds, Cmp::SpriteAnimation>();
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
      new_pos.position.x = pos_cmp.position.x + ( dir_cmp.x * BaseSystem::kGridSquareSizePixels.x );
      new_pos.position.y = pos_cmp.position.y + ( dir_cmp.y * BaseSystem::kGridSquareSizePixels.y );

      // Check collision ONCE
      bool can_move = skip_collision_check || is_valid_move( new_pos );
      if ( !can_move ) continue; // Early exit if blocked

      // Check if moving diagonally AFTER we know movement is valid
      bool is_diagonal = ( dir_cmp.x != 0.0f ) && ( dir_cmp.y != 0.0f );
      bool diagonal_between_obstacles = is_diagonal && isDiagonalMovementBetweenObstacles( pos_cmp, dir_cmp );

      auto &player_lerp_speed = get_persistent_component<Cmp::Persistent::PlayerLerpSpeed>();
      auto &diagonal_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
      auto &shortcut_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();

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
  const auto viewBounds = BaseSystem::calculate_view_bounds( RenderSystem::getGameView() );

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_db_cmp] : player_view.each() )
  {
    auto add_path_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::NoPathFinding> );
    for ( auto [path_entt, path_pos_cmp] : add_path_view.each() )
    {
      if ( pc_db_cmp.findIntersection( path_pos_cmp ) )
      {
        if ( !is_visible_in_view( viewBounds, path_pos_cmp ) ) continue; // optimization

        // calculate the distance from the position to the player
        auto distance = std::floor( getEuclideanDistance( pc_pos_cmp.position, path_pos_cmp.position ) );
        getReg().emplace_or_replace<Cmp::PlayerDistance>( path_entt, distance );
      }
    }

    auto remove_path_view = getReg().view<Cmp::Position>();
    for ( auto [path_entt, path_pos_cmp] : remove_path_view.each() )
    {
      if ( not pc_db_cmp.findIntersection( path_pos_cmp ) )
      {
        // tidy up any out of range obstacles
        getReg().remove<Cmp::PlayerDistance>( path_entt );
      }
    }
  }
}

void PlayerSystem::checkPlayerMortality()
{
  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality>();
  for ( auto [entity, pc_cmp, health_cmp, mortality_cmp] : player_view.each() )
  {
    if ( health_cmp.health <= 0 )
    {
      mortality_cmp.state = Cmp::PlayerMortality::State::DEAD;
      SPDLOG_INFO( "Player has progressed to deadness." );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::GAME_OVER );
    }
  }
}

} // namespace ProceduralMaze::Sys