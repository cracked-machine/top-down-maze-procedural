#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRenderOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/AnimData.hpp>
#include <Components/ArrowProjectile.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Inventory/Explosive.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/LastDirection.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Moveable.hpp>
#include <Components/NoMoveDest.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerMovementSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/PostPullMovementDelay.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Player/TorchRadius.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/Ruin/RuneMarking.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/Spring/HealingSpringMultiBlock.hpp>
#include <Components/Stats/BaseAction.hpp>
#include <Components/Stats/CarryAction.hpp>
#include <Components/Stats/CollisionAction.hpp>
#include <Components/Stats/DestroyAction.hpp>
#include <Components/Stats/PlayerStats.hpp>
#include <Components/Stats/ProjectileAction.hpp>
#include <Components/Stats/ProximityAction.hpp>
#include <Components/Stats/SpawnAction.hpp>
#include <Components/System.hpp>
#include <Components/UUID.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <Components/Wall.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>
#include <string>

namespace Game::Sys
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
  update_player_no_path_cmp( dt );

  // cache position so we can update player in spatial grid after changes.
  auto old_player_pos = Utils::Player::get_position( reg() );

  fade_player_on_wormhole_jump();
  blink_player();

  if ( not m_post_death_timer.isRunning() )
  {
    check_player_can_push( dt );
    check_player_can_pull( dt );
    update_player_position( dt, Utils::get_system_cmp( reg() ).collisions_disabled );
    update_player_animation();

    Utils::Player::get_zorder( reg() ).setZOrder( Utils::Player::get_position( reg() ).position.y );
    Utils::Player::get_direction( reg() ) == sf::Vector2f( 0.f, 0.f ) ? stop_footsteps_sound() : play_footsteps_sound( footstep_sfx );
  }

  check_player_mortality();
  check_timed_action_side_effects( dt );

  if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_npc_navmesh.lock() )
  {
    pathfinding_navmesh->update( Utils::Player::get_entity( reg() ), old_player_pos, Utils::Player::get_position( reg() ) );
  }
  if ( PathFinding::SpatialHashGridSharedPtr open_navmesh = m_open_navmesh.lock() )
  {
    open_navmesh->update( Utils::Player::get_entity( reg() ), old_player_pos, Utils::Player::get_position( reg() ) );
  }

  update_arrow_trajectory( dt );
}

void PlayerSystem::play_footsteps_sound( FootStepSfx type )
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
      break;
    }
  }
}

void PlayerSystem::stop_footsteps_sound()
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

void PlayerSystem::force_expire_damage_cooldown()
{
  for ( auto [player_entt, player_cmp] : reg().view<Cmp::PlayerCharacter>().each() )
  {
    player_cmp.skip_damage_cooldown_once = true;
  }
}

void PlayerSystem::move_obstacle( const sf::FloatRect &target_position )
{
  // check if player can move the obstacle
  for ( auto [selected_entt, selected_cmp, moveable_cmp, selected_pos_cmp, selected_uuid_cmp] :
        reg().view<Cmp::SelectedPosition, Cmp::Moveable, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( not target_position.findIntersection( selected_pos_cmp ) ) continue;

    // normalized direction
    auto player_direction = Utils::Player::get_direction( reg() );
    // velocity vector
    auto player_velocity = Cmp::RectBounds::scaled( player_direction.componentWiseMul( Constants::kGridSizePxF ), target_position.size, 1 );
    // where we want the player to end up after the move (used for pull only)
    auto player_dest_position = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ).position + player_velocity.position(),
                                                         Constants::kGridSizePxF, 1 );
    // where we want the obstacle to end up after the move (push and pull)
    auto obstacle_dest_position = Cmp::RectBounds::scaled( selected_pos_cmp.position + player_velocity.position(), Constants::kGridSizePxF, 1 );

    bool new_position_is_empty = true;
    new_position_is_empty = not Utils::Collision::check_cmp<Cmp::Obstacle>( reg(), obstacle_dest_position ) and
                            not Utils::Collision::check_cmp<Cmp::NoMoveDest>( reg(), obstacle_dest_position );

    // can we move player in the opposite direction if they are pulling?
    bool player_in_the_way = Utils::Collision::check_cmp<Cmp::PlayerCharacter>( reg(), obstacle_dest_position );
    if ( player_in_the_way )
    {
      if ( ( Utils::Collision::check_cmp<Cmp::Obstacle>( reg(), player_dest_position ) ) or
           ( Utils::Collision::check_cmp<Cmp::NoMoveDest>( reg(), player_dest_position ) ) )
      {
        // if not then cancel the move
        new_position_is_empty = false;
      }
    }

    // move the player and the obstacle
    if ( new_position_is_empty )
    {
      Utils::Player::get_position( reg() ).position = player_dest_position.position();

      m_sound_bank.get_effect( "crypt_open" ).play();

      // move the obstacle
      selected_pos_cmp.position += player_velocity.position();
      reg().remove<Cmp::SelectedPosition>( selected_entt );

      // find the matching cap to this obstacle and move it too
      for ( auto [cap_search_entt, cap_search_uuid_cmp, cap_search_pos_cmp] :
            reg().view<Cmp::UUID, Cmp::Position>( entt::exclude<Cmp::Obstacle> ).each() )
      {
        if ( cap_search_uuid_cmp != selected_uuid_cmp ) continue;
        cap_search_pos_cmp.position += player_velocity.position();
      }

      // if we moved obstacle into cobweb it is now stuck :'(
      for ( auto [cobweb_entt, cobweb_cmp, cobweb_pos_cmp] : reg().view<Cmp::RuinCobweb, Cmp::Position>().each() )
      {
        if ( selected_pos_cmp.findIntersection( cobweb_pos_cmp ) ) { reg().remove<Cmp::Moveable>( selected_entt ); }
      }

      // if we moved obstacle onto rune marking then set it as activated.
      auto rune_view = reg().view<Cmp::RuneMarking, Cmp::Position, Cmp::ZOrderValue, Cmp::AnimData, Cmp::UUID>();
      for ( auto [rune_entt, rune_cmp, rune_pos_cmp, rune_zorder_cmp, rune_anim_cmp, rune_uuid_cmp] : rune_view.each() )
      {
        // Check if ANY obstacle (not just the moved one) is on this rune.
        // Undo the visual y offset so intersection uses the logical rune position —
        // without this, a block sliding up leaves an 8px phantom overlap that
        // prevents deactivation.
        float check_offset = rune_cmp.active ? 8.f : 0.f;
        rune_pos_cmp.position.y += check_offset;

        bool any_obstacle_on_rune = false;
        auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position>();
        for ( auto [obs_entt, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
        {
          if ( obstacle_pos_cmp.findIntersection( rune_pos_cmp ) )
          {
            any_obstacle_on_rune = true;
            break;
          }
        }

        rune_pos_cmp.position.y -= check_offset;

        if ( any_obstacle_on_rune != rune_cmp.active )
        {
          const std::string sprite_type = any_obstacle_on_rune ? "sprite.ruin.runemarking.active" : "sprite.ruin.runemarking.inactive";

          if ( any_obstacle_on_rune )
          {
            Factory::Particle::add_rune_ps( m_reg, "ruin.rune.particles", 1.f, 10.f, rune_uuid_cmp,
                                            { rune_pos_cmp.position.x + 8.f, rune_pos_cmp.position.y }, 5000 );
          }
          else
          {
            for ( auto [ps_entt, ps_cmp, ps_uuid_cmp] : reg().view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
            {
              if ( ps_cmp.sprite->get_tag() == "ruin.rune.particles" and ps_uuid_cmp == rune_uuid_cmp ) reg().destroy( ps_entt );
            }
          }
          // shift the rune on the y-axis to adjust for the front-facing perspective
          if ( any_obstacle_on_rune ) { rune_pos_cmp.position.y -= 8.f; }
          else { rune_pos_cmp.position.y += 8.f; }

          float zorder = m_sprite_factory.get_spritesheet_by_type( sprite_type ).get_zorder( 0 );
          rune_zorder_cmp.setZOrder( zorder );
          rune_anim_cmp.m_sprite_type = sprite_type;
          rune_cmp.active = any_obstacle_on_rune;
        }
      }
      m_movement_suppress_clock.restart();
      break;
    }
  }
}

void PlayerSystem::check_player_can_push( sf::Time dt )
{
  auto movement_delay = Sys::PersistSystem::get<Cmp::Persist::PostPullMovementDelay>( reg() );
  if ( m_movement_suppress_clock.getElapsedTime().asSeconds() < movement_delay.get_value() ) return;

  const Cmp::Direction raw_direction = Utils::Player::get_direction( reg() );
  if ( raw_direction == sf::Vector2f( 0.f, 0.f ) ) return; // optimization
  auto &player_movement_speed = Sys::PersistSystem::get<Cmp::Persist::PlayerMovementSpeed>( reg() );
  const float step = player_movement_speed.get_value() * dt.asSeconds();
  const Cmp::Direction direction = raw_direction.componentWiseMul( { step, step } );

  const auto player_pos = Utils::Player::get_position( reg() );
  const sf::FloatRect next_horizontal_move( { player_pos.position.x + direction.x, player_pos.position.y }, player_pos.size );
  move_obstacle( next_horizontal_move );
  const sf::FloatRect next_vertical_move( { player_pos.position.x, player_pos.position.y + direction.y }, player_pos.size );
  move_obstacle( next_vertical_move );
}
void PlayerSystem::check_player_can_pull( sf::Time dt )
{
  auto movement_delay = Sys::PersistSystem::get<Cmp::Persist::PostPullMovementDelay>( reg() );
  if ( m_movement_suppress_clock.getElapsedTime().asSeconds() < movement_delay.get_value() ) return;

  const Cmp::Direction raw_direction = Utils::Player::get_direction( reg() );
  if ( raw_direction == sf::Vector2f( 0.f, 0.f ) ) return; // optimization
  auto &player_movement_speed = Sys::PersistSystem::get<Cmp::Persist::PlayerMovementSpeed>( reg() );
  const float step = player_movement_speed.get_value() * dt.asSeconds();
  const Cmp::Direction direction = raw_direction.componentWiseMul( { step, step } );

  const auto player_pos = Utils::Player::get_position( reg() );
  const sf::FloatRect prev_horizontal_move( { player_pos.position.x - direction.x, player_pos.position.y }, player_pos.size );
  move_obstacle( prev_horizontal_move );
  const sf::FloatRect prev_vertical_move( { player_pos.position.x, player_pos.position.y - direction.y }, player_pos.size );
  move_obstacle( prev_vertical_move );
}

void PlayerSystem::update_player_position( sf::Time dt, bool collision_disabled )
{
  auto movement_delay = Sys::PersistSystem::get<Cmp::Persist::PostPullMovementDelay>( reg() );
  if ( m_movement_suppress_clock.getElapsedTime().asSeconds() < movement_delay.get_value() ) return;

  Cmp::Position &player_pos = Utils::Player::get_position( reg() );

  const Cmp::Direction raw_direction = Utils::Player::get_direction( reg() );
  if ( raw_direction == sf::Vector2f( 0.f, 0.f ) ) return; // optimization

  auto &player_movement_speed = Sys::PersistSystem::get<Cmp::Persist::PlayerMovementSpeed>( reg() );
  float penalised_movement_speed = player_movement_speed.get_value() * Utils::Player::get_speed_penalty( reg() );
  const float step = penalised_movement_speed * dt.asSeconds();
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

  auto movement_delay = Sys::PersistSystem::get<Cmp::Persist::PostPullMovementDelay>( reg() );
  if ( m_movement_suppress_clock.getElapsedTime().asSeconds() < movement_delay.get_value() ) return;

  const Cmp::Direction direction_cmp = Utils::Player::get_direction( reg() );
  Cmp::AnimData &anim_cmp = Utils::Player::get_sprite_anim( reg() );

  // update the animation state based on movement direction
  if ( direction_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { anim_cmp.m_enabled = false; }
  else
  {
    anim_cmp.m_enabled = true;
    if ( direction_cmp.x == 1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.east"; }
    else if ( direction_cmp.x == -1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.west"; }
    else if ( direction_cmp.y == -1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.north"; }
    else if ( direction_cmp.y == 1 ) { anim_cmp.m_sprite_type = "sprite.player.walk.south"; }

    // store the direction
    if ( direction_cmp.x != 0.f || direction_cmp.y != 0.f )
    {
      auto &last_dir = Utils::Player::get_last_direction( reg() );
      if ( std::abs( direction_cmp.x ) >= std::abs( direction_cmp.y ) ) { last_dir = sf::Vector2f{ direction_cmp.x > 0.f ? 1.f : -1.f, 0.f }; }
      else { last_dir = sf::Vector2f{ 0.f, direction_cmp.y > 0.f ? 1.f : -1.f }; }
    }
  }
}

void PlayerSystem::update_arrow_trajectory( sf::Time dt )
{

  for ( auto [arrow_entt, arrow_cmp, arrow_pos_cmp] : reg().view<Cmp::ArrowProjectile, Cmp::Position>().each() )
  {
    if ( arrow_cmp.m_fixed_time_step_accumulator >= arrow_cmp.fixed_time_step_max() )
    {
      if ( arrow_cmp.m_in_flight )
      {
        // Arrow still in flight; update the arrow position/angle using its latest direction
        sf::Vector2f remaining = arrow_cmp.m_destination - arrow_pos_cmp.getCenter();
        if ( remaining.length() <= arrow_cmp.speed() ) { arrow_pos_cmp.position = arrow_cmp.m_destination - arrow_pos_cmp.size / 2.f; }
        else if ( auto direction = Utils::Maths::normalized( remaining ) )
        {
          arrow_pos_cmp.position += *direction * arrow_cmp.speed();
          reg().emplace_or_replace<Cmp::AbsoluteRotation>( arrow_entt, ( *direction ).angle().asDegrees() );
        }

        // Displace the sprite along a parabolic arc that peaks halfway between origin and destination.
        // The underlying Position stays on the straight line so NPC hit detection is unaffected.
        // The arc is drawn perpendicular to the flight path: a vertical bulge for a mostly-horizontal
        // shot, a horizontal bulge for a mostly-vertical shot, so it stays visible either way.
        static const float kParabolaHeight = 0.5f;
        sf::Vector2f flight_vector = arrow_cmp.m_destination - arrow_cmp.m_origin;
        float total_distance = flight_vector.length();
        if ( total_distance > 0.f )
        {
          float progress = std::clamp( ( arrow_pos_cmp.getCenter() - arrow_cmp.m_origin ).length() / total_distance, 0.f, 1.f );
          float arc_magnitude = -kParabolaHeight * arrow_cmp.arc_height() * progress * ( 1.f - progress );
          sf::Vector2f arc_offset = std::abs( flight_vector.x ) >= std::abs( flight_vector.y ) ? sf::Vector2f{ 0.f, arc_magnitude }
                                                                                               : sf::Vector2f{ arc_magnitude, 0.f };
          reg().emplace_or_replace<Cmp::AbsoluteRenderOffset>( arrow_entt, arc_offset );
        }
      }

      // Checked unconditionally (not just when already landed) so that the impact sound/effects fire
      // on the exact same tick the arrow snaps to its destination above, instead of one tick later.
      if ( arrow_pos_cmp.getCenter() == arrow_cmp.m_destination )
      {
        // arrow has reached destination
        if ( arrow_cmp.m_in_flight )
        {
          m_sound_bank.get_effect( std::string( "arrow_hit" + std::to_string( Cmp::RandomInt( 1, 4 ).gen() ) ) ).play();
          m_sound_bank.get_effect( "draw_bow" ).stop();
          m_sound_bank.get_effect( "release_bow" ).stop();
          arrow_cmp.m_in_flight = false;
          if ( auto *rotation_cmp = reg().try_get<Cmp::AbsoluteRotation>( arrow_entt ) ) arrow_cmp.m_rest_angle = rotation_cmp->getAngle();

          // Re-pivot rotation around the arrowhead (tip) instead of the sprite centre, so the impact
          // point stays fixed in place while the fletching end wiggles around it. AbsoluteOffset is a
          // local pivot in sprite space (tip = far edge along the sprite's default facing direction);
          // AbsoluteRenderOffset compensates in world space so the tip lands exactly where the sprite's
          // centre used to be, accounting for whatever angle the arrow was flying at on impact -
          // otherwise the sprite visibly snaps sideways the instant it switches pivot.
          float rest_rad = sf::degrees( arrow_cmp.m_rest_angle ).asRadians();
          float half_width = arrow_pos_cmp.size.x / 2.f;
          sf::Vector2f tip_from_centre{ std::cos( rest_rad ) * half_width, std::sin( rest_rad ) * half_width };
          reg().emplace_or_replace<Cmp::AbsoluteOffset>( arrow_entt, sf::Vector2f{ arrow_pos_cmp.size.x, arrow_pos_cmp.size.y / 2.f } );
          reg().emplace_or_replace<Cmp::AbsoluteRenderOffset>( arrow_entt, tip_from_centre - sf::Vector2f{ half_width, 0.f } );
        }

        arrow_cmp.m_landed_elapsed += dt;

        // Destroy spent arrows a while after they land so they don't pile up.
        static const sf::Time kDestroyDelay = sf::seconds( 5.f );
        if ( arrow_cmp.m_landed_elapsed >= kDestroyDelay )
        {
          reg().destroy( arrow_entt );
          continue;
        }

        // Wiggle the arrow briefly on impact, decaying back to its resting angle.
        static const sf::Time kWiggleDuration = sf::seconds( 0.2f );
        static const float kWiggleFrequencyHz = 100.f;
        static const float kWiggleAmplitudeDeg = 1.f;
        if ( arrow_cmp.m_landed_elapsed < kWiggleDuration )
        {
          float decay = 1.f - ( arrow_cmp.m_landed_elapsed.asSeconds() / kWiggleDuration.asSeconds() );
          float wiggle_deg = kWiggleAmplitudeDeg * decay * std::sin( arrow_cmp.m_landed_elapsed.asSeconds() * kWiggleFrequencyHz );
          reg().emplace_or_replace<Cmp::AbsoluteRotation>( arrow_entt, arrow_cmp.m_rest_angle + wiggle_deg );
        }
      }

      arrow_cmp.m_fixed_time_step_accumulator = sf::Time::Zero;
    }
    else { arrow_cmp.m_fixed_time_step_accumulator += dt; }
  }
}

void PlayerSystem::check_player_mortality()
{

  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::PlayerMortality, Cmp::Position>();
  for ( auto [entity, pc_cmp, mortality_cmp, player_pos_cmp] : player_view.each() )
  {
    if ( ( mortality_cmp.state == Cmp::PlayerMortality::State::DEAD ) and ( m_post_death_timer.getElapsedTime() > sf::seconds( 5.f ) ) )
    {
      if ( Utils::Player::player_has_extra_life( reg() ) )
      {
        Utils::Player::get_position( reg() ).position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( reg() );
        Factory::Player::remove_player_extra_life( reg() );
        m_sound_bank.get_effect( "player_respawn" ).play();
        Utils::Player::get_player_stats( reg() ).apply_modifiers( { Cmp::Stats::Health{ 100 }, {}, {}, {}, {}, {} } );
        Utils::Player::get_mortality( reg() ).state = Cmp::PlayerMortality::State::ALIVE;
        Utils::Player::get_zorder( reg() ).setZOrder( Utils::Player::get_position( reg() ).y() );
        m_post_death_timer.reset();
      }
      else
      {
        // reg().remove<Cmp::AnimData>( Utils::Player::get_entity( reg() ) );
        SPDLOG_DEBUG( "Player has progressed to deadness." );
        m_post_death_timer.reset();
        stop_footsteps_sound();

        m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::GAME_OVER );
      }
    }
  }
}

void PlayerSystem::check_timed_action_side_effects( sf::Time dt )
{
  // To prevent the individual modifiers from fighting with each other, we need to sum them first, then apply that sum at the end of this function.
  // Therefore the summing of each modifier should be done in sync with a 1 second tick (kTimedActionSyncThreshold).
  // Because PlayerInventorySlot/NPC components have independent timers from the light/dark timer, we need to update their timers every frame.

  Cmp::BaseAction net_modifier( {}, {}, {}, {}, {}, {} );
  std::stringstream mod_log;
  const auto candle_item = Sys::ItemStore::instance().get_item( "item.candle" );

  update_timed_action_clocks( dt );

  // Now accumulate the PlayerInventorySlot/NPC/DarknessFear values if their clocks are expired.
  static constexpr float kTimedActionSyncClockMax = 1.0;
  m_timed_action_sync_clock += dt;
  if ( m_timed_action_sync_clock.asSeconds() >= kTimedActionSyncClockMax )
  {
    // add the NPC modifiers to the `net_modifier` every kTimedActionSyncClockMax.
    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : reg().view<Cmp::NPC, Cmp::Position>().each() )
    {
      mod_log << " " << npc_cmp.sprite_type_list.front() << "(actions";
      for ( auto &[action_type, npc_action_pair] : npc_cmp.actions )
      {
        // These are handled as one time modifiers handled by specific systems/factories. Note the tick action field is ignored.
        if ( action_type == std::type_index( typeid( Cmp::CollisionAction ) ) ) { continue; }  // See NpcSystem
        if ( action_type == std::type_index( typeid( Cmp::ProjectileAction ) ) ) { continue; } // See ShockwaveSystem
        if ( action_type == std::type_index( typeid( Cmp::SpawnAction ) ) ) { continue; }      // See NpcFactory/GraveSystem
        if ( action_type == std::type_index( typeid( Cmp::DestroyAction ) ) ) { continue; }    // See NpcFactory

        // special case: Only apply ProximityAction when the NPC is in the current screen view.
        if ( action_type == std::type_index( typeid( Cmp::ProximityAction ) ) and
             not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), npc_pos_cmp ) )
        {
          continue;
        }
        auto &[npc_action, npc_action_timer] = npc_action_pair;

        if ( npc_action_timer.asSeconds() < npc_action.interval() ) continue;
        net_modifier += npc_action;
        mod_log << "[" << npc_action.health() << "," << npc_action.fear() << "," << npc_action.despair() << "," << npc_action.infamy() << "]";
        npc_action_timer = sf::Time::Zero;
      }
      mod_log << ")";
    }

    // get the DarknessFear tick interval from the candle item in res/json/items.json
    const static float kDarknessFearClockMax = candle_item.actions.at( std::type_index( typeid( Cmp::CarryAction ) ) ).action.interval();
    if ( m_darkness_fear_clock.asSeconds() >= kDarknessFearClockMax )
    {
      Cmp::BaseAction fear_of_the_dark( {}, { +1 }, {}, {}, {}, {} );
      net_modifier += fear_of_the_dark;
      mod_log << " dark[" << fear_of_the_dark.fear() << "]";

      auto torch_radius = Utils::Player::get_torch_radius( reg() );
      for ( auto [candle_entt, candle_cmp, candle_pos] : reg().view<Cmp::WorldItem, Cmp::Position>().each() )
      {
        if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), candle_pos ) ) continue;
        if ( not candle_cmp.sprite_type.contains( "candle" ) ) continue;
        float player_distance = Utils::Maths::getEuclideanDistance( candle_pos.getCenter(), Utils::Player::get_position( reg() ).position );
        if ( player_distance > torch_radius.value ) continue;

        for ( auto &[action_type, item_action_pair] : candle_cmp.actions )
        {
          if ( action_type == std::type_index( typeid( Cmp::CarryAction ) ) )
          {
            auto &[item_action, item_action_timer] = item_action_pair;
            net_modifier += item_action;
            mod_log << " light[" << item_action.fear() << "]";
            item_action_timer = sf::Time::Zero;
          }
        }
      }

      // add the item modifiers to the `net_modifier` in sync with the `m_darkness_fear_clock` to prevent racing.
      for ( auto [slot_entt, slot_cmp] : reg().view<Cmp::PlayerInventorySlot>().each() )
      {
        for ( auto &[action_type, item_action_pair] : slot_cmp.m_item.actions )
        {
          if ( action_type != std::type_index( typeid( Cmp::CarryAction ) ) ) continue;
          auto &[item_action, item_action_timer] = item_action_pair;
          if ( item_action.fear() == 0 ) continue;
          net_modifier += item_action;
          mod_log << " inv_light[" << item_action.fear() << "]";
          item_action_timer = sf::Time::Zero;
        }
      }

      // apply candle item modifiers to the player when standing inside flame radius of altar
      const static auto candle_action_modifiers = candle_item.actions.at( std::type_index( typeid( Cmp::CarryAction ) ) ).action;
      for ( auto [altar_entt, altar_cmp, altar_uuid_cmp] : reg().view<Cmp::AltarMultiBlock, Cmp::UUID>().each() )
      {
        if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), altar_cmp ) ) continue;
        for ( auto [particle_entt, particle_cmp, particle_uuid_cmp] : reg().view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
        {
          if ( altar_uuid_cmp != particle_uuid_cmp ) continue;

          float player_distance = Utils::Maths::getEuclideanDistance( particle_cmp.sprite->get_emitter_position(),
                                                                      Utils::Player::get_position( reg() ).position );
          if ( player_distance > torch_radius.value ) continue;
          net_modifier += candle_action_modifiers;
        }
      }

      // healing spring
      Cmp::BaseAction fountain_effects( { +5 }, { -5 }, { -5 }, { -5 }, { -5 }, {} );
      for ( auto [fountain_entt, fountain_mb_cmp] : reg().view<Cmp::HealingSpringMultiBlock>().each() )
      {
        if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), fountain_mb_cmp ) ) continue;

        float player_distance = Utils::Maths::getEuclideanDistance( fountain_mb_cmp.position, Utils::Player::get_position( reg() ).position );
        if ( player_distance > 500 ) continue;
        net_modifier += fountain_effects;
      }

      // apply candle item modifiers to the player when standing inside radius of wisp NPC
      for ( auto [altar_entt, npc_cmp, npc_pos_cmp] : reg().view<Cmp::NPC, Cmp::Position>().each() )
      {
        if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;
        if ( not npc_cmp.sprite_type_list.front().contains( "wisp" ) ) continue;

        float player_distance = Utils::Maths::getEuclideanDistance( npc_pos_cmp.getCenter(), Utils::Player::get_position( reg() ).position );
        if ( player_distance > torch_radius.value ) continue;
        net_modifier += candle_action_modifiers;
      }

      m_darkness_fear_clock = sf::Time::Zero;
    }
    m_timed_action_sync_clock = sf::Time::Zero;
    SPDLOG_DEBUG( "modifiers: {}, total: {}", mod_log.str(), net_modifier.fear() );

    check_player_max_fear_despair();
  }
  Utils::Player::get_player_stats( reg() ).apply_modifiers( net_modifier );
}

void PlayerSystem::update_timed_action_clocks( sf::Time dt )
{

  // update PlayerInventorySlot/NPC/DarknessFear clock every frame.
  for ( auto [slot_entt, slot_cmp] : reg().view<Cmp::PlayerInventorySlot>().each() )
  {
    for ( auto &[action_type, item_action_pair] : slot_cmp.m_item.actions )
    {
      if ( action_type == std::type_index( typeid( Cmp::CollisionAction ) ) ) continue;
      if ( action_type == std::type_index( typeid( Cmp::ProjectileAction ) ) ) continue;
      auto &[item_action, item_action_timer] = item_action_pair;
      if ( item_action.interval() == 0.f ) continue;
      item_action_timer += dt;
      SPDLOG_DEBUG( "PlayerInventorySlot item_action_timer {}", item_action_timer.asSeconds() );
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
      SPDLOG_DEBUG( "NPC npc_action_timer {}", npc_action_timer.asSeconds() );
    }
  }
  m_darkness_fear_clock += dt;
}

void PlayerSystem::check_player_max_fear_despair()
{
  // check if player should take health damage/die
  if ( Utils::Player::get_player_stats( reg() ).fear() == 100 )
  {
    Utils::Player::get_player_stats( reg() ).apply_modifiers( Cmp::BaseAction( { -1 }, {}, {}, {}, {}, {} ) );
    if ( Utils::Player::get_player_stats( reg() ).health() == 0 and Utils::Player::get_mortality( reg() ).state != Cmp::PlayerMortality::State::DEAD )
    {
      on_player_mortality_event( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::TERRIFIED, Utils::Player::get_position( reg() ) ) );
    }
  }
  else if ( Utils::Player::get_player_stats( reg() ).despair() == 100 and
            Utils::Player::get_mortality( reg() ).state != Cmp::PlayerMortality::State::DEAD )
  {
    on_player_mortality_event( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SUICIDE, Utils::Player::get_position( reg() ) ) );
  }
}

void PlayerSystem::check_player_axe_npc_kill()
{
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_npc_navmesh.lock();
  if ( not pathfinding_navmesh ) return;

  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( inventory_slot_type != "sprite.item.axe" ) { return; }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // Remove any existing SelectedPosition from NPCs only — this runs every frame the attack button is
  // held with no cooldown of its own, so clearing the whole registry's SelectedPosition here would
  // also wipe unrelated selections (e.g. the obstacle currently being dug) set by other systems
  auto selected_position_view = reg().view<Cmp::SelectedPosition, Cmp::NPC>();
  for ( auto [existing_sel_entity, sel_cmp, npc_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = reg().view<Cmp::Position, Cmp::NPC, Cmp::AnimData>( entt::exclude<Cmp::SelectedPosition> );
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
          auto dropped_loot_entt = Factory::Loot::create_loot_drop(
              reg(), Cmp::AnimData( Cmp::AnimData::Config{ .sprite_type = sprite_type, .enabled = false } ),
              Cmp::RectBounds::scaled( npc_pos_cmp.position, npc_pos_cmp.size, 2.f ).getBounds(), Factory::IncludePack<>{},
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
          Factory::Npc::destroy_npc( reg(), npc_entity );
        }
      }

      SPDLOG_DEBUG( "Dug through obstacle at position ({}, {})!", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
    }
  }
}

void PlayerSystem::check_player_fire_arrow( float charge_fraction )
{
  auto mouse_pos = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
  SPDLOG_INFO( "Firing arrow to {},{}", mouse_pos.position.x, mouse_pos.position.y );

  auto arrow_entt = reg().create();
  auto arrow_origin = Utils::Player::get_position( reg() );
  reg().emplace_or_replace<Cmp::Position>( arrow_entt, arrow_origin.position, arrow_origin.size );

  // Charge scales how far along the origin->mouse line the arrow actually travels;
  // a weak draw drops the arrow short instead of always reaching the cursor.
  sf::Vector2f origin_center = arrow_origin.getCenter();
  sf::Vector2f arrow_destination = origin_center + ( mouse_pos.getCenter() - origin_center ) * charge_fraction;

  reg().emplace_or_replace<Cmp::ArrowProjectile>( arrow_entt, origin_center, arrow_destination );
  reg().emplace_or_replace<Cmp::ZOrderValue>( arrow_entt, 50000 );
  reg().emplace_or_replace<Cmp::AbsoluteOffset>( arrow_entt, arrow_origin.size / 2.f );
  if ( auto angle = Utils::Maths::angle( mouse_pos.getCenter() - arrow_origin.getCenter() ) )
  {
    reg().emplace_or_replace<Cmp::AbsoluteRotation>( arrow_entt, angle->asDegrees() );
  }
  // clang-format off
    reg().emplace_or_replace<Cmp::AnimData>( arrow_entt, Cmp::AnimData::Config{
          .sprite_type = "sprite.item.arrow",
          .frame_index_offset = 0,
          .enabled = true
    });
  // clang-format on
}

void PlayerSystem::fade_player_on_wormhole_jump()
{
  auto player_entt = Utils::Player::get_entity( reg() );
  auto *wormhole_jump = reg().try_get<Cmp::WormholeJump>( player_entt );
  if ( wormhole_jump )
  {
    // Calculate fade based on elapsed time vs total cooldown
    float elapsed = wormhole_jump->jump_clock.getElapsedTime().asSeconds();
    float cooldown = wormhole_jump->jump_cooldown.asSeconds();
    float progress = std::min( elapsed / cooldown, 1.0f ); // 0.0 to 1.0
    Utils::Player::get_alpha( reg() ).setAlpha( static_cast<uint8_t>( 255 * ( 1.0f - progress ) ) );
  }
}

void PlayerSystem::blink_player()
{
  // damage cooldown blink effect
  for ( auto [player_entt, player_cmp] : reg().view<Cmp::PlayerCharacter>().each() )
  {

    auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( reg() );
    bool is_in_damage_cooldown = player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
    bool blink_visible = ( player_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;

    auto &alpha_cmp = Utils::Player::get_alpha( reg() );
    if ( not is_in_damage_cooldown or player_cmp.skip_damage_cooldown_once or ( is_in_damage_cooldown and blink_visible ) ) { alpha_cmp = 255; }
    else { alpha_cmp = 0; }
  }
}

void PlayerSystem::drop_inventory_slot_into_world( sf::Vector2f pos, entt::entity inventory_slot_entt )
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
    // multiblocks are top-left anchored, so offset the y-axis so that plant base is at players feet
    auto [mb_entt, segment_entt_list] = Factory::Multiblock::add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>(
        reg(), Utils::snap_to_grid( { pos.x, pos.y - Constants::kGridSizePxF.y } ),
        m_sprite_factory.get_spritesheet_by_type( inventory_slot_cmp->m_item.sprite_type ) );

    // rebuild the m_player_navmesh here
    if ( auto player_navmesh = m_player_navmesh.lock() )
    {
      player_navmesh->clear();
      for ( auto [entt, nopath_cmp, pos_cmp] : reg().view<Cmp::PlayerNoPath, Cmp::Position>().each() )
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
        if ( not reg().any_of<Cmp::NpcNoPathFinding>( seg_entt ) ) continue;
        auto seg_pos_cmp = reg().get<Cmp::Position>( seg_entt );
        for ( auto blocked_entt : npc_navmesh->at( seg_pos_cmp ) )
        {
          if ( reg().any_of<Cmp::PlayerCharacter>( blocked_entt ) ) continue;
          npc_navmesh->remove( blocked_entt, seg_pos_cmp );
        }
      }
    }

    // clear player inevntory
    reg().destroy( inventory_slot_entt );
    m_sound_bank.get_effect( "chopping_final" ).play();
    return;
  }

  // otherwise just drop it as a Re-pickupable item
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
  reg().emplace_or_replace<Cmp::NpcNoPathFinding>( world_item_entt );

  // try to copy any relevant components over to the new world carryitem entt
  auto *inventory_slot_level_cmp = reg().try_get<Cmp::InventoryWearLevel>( inventory_slot_entt );
  if ( inventory_slot_level_cmp ) { reg().emplace_or_replace<Cmp::InventoryWearLevel>( world_item_entt, inventory_slot_level_cmp->m_level ); }

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
        // Put the flame particle just behind the players zorder
        reg().emplace_or_replace<Cmp::ZOrderValue>( ps_entt, Utils::Player::get_position(reg()).y() - 1);
      }
    }
    reg().emplace_or_replace<Cmp::UUID>( world_item_entt, uuid_cmp->data );
  }

  // now destroy the inventory slot
  reg().destroy( inventory_slot_entt );
  if ( world_item_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
}

void PlayerSystem::pickup_world_item( entt::registry &reg, entt::entity world_item_entt )
{
  
  auto *anim_data_cmp = reg.try_get<Cmp::AnimData>( world_item_entt );
  if ( not anim_data_cmp ) return;

  auto sprite_key = anim_data_cmp->m_sprite_type;
  if ( sprite_key.starts_with( "sprite." ) )
    sprite_key = sprite_key.substr( 7 );
  auto world_item_cmp = Sys::ItemStore::instance().get_item( sprite_key );  

  // create the basic inventory slot entt
  auto inventory_entity = reg.create();
  reg.emplace_or_replace<Cmp::PlayerInventorySlot>( inventory_entity, world_item_cmp );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( inventory_entity, Cmp::AnimData::Config{  
        .sprite_type =  world_item_cmp.sprite_type,
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
        // Move the ParticleSprite to the UI view. Any particle sprites should be fully cleared
        // otherwise we get particle effects in strange places during the transition frame.
        // The emitter position is set by RenderGameSystem using the UiData object.
        ps_owner.sprite->clear();
        ps_owner.sprite->set_view_type( Cmp::Particle::ViewType::SCREEN );
        // The flame particle needs to be in front of everything when in the UI panel
        reg.emplace_or_replace<Cmp::ZOrderValue>( ps_entt, 50000 );
      }
    }
    reg.emplace_or_replace<Cmp::UUID>( inventory_entity, uuid_cmp->data );
  }

  auto *wear_level_cmp = reg.try_get<Cmp::InventoryWearLevel>( world_item_entt );
  if ( wear_level_cmp ) { reg.emplace_or_replace<Cmp::InventoryWearLevel>( inventory_entity, wear_level_cmp->m_level ); }

  auto *scryingball_cmp = reg.try_get<Cmp::SeeingStone>( world_item_entt );
  if ( scryingball_cmp ) { reg.emplace_or_replace<Cmp::SeeingStone>( inventory_entity, false, scryingball_cmp->target ); }

  auto *explosive_cmp = reg.try_get<Cmp::Explosive>( world_item_entt );
  if ( explosive_cmp ) { reg.emplace_or_replace<Cmp::Explosive>( inventory_entity, false ); }

  // now destroy the world item entt
  SPDLOG_DEBUG( "Picked up world entt {}", static_cast<uint32_t>( world_item_entt ) );
  Factory::Plant::remove_plant_mb( reg, world_item_entt, m_npc_navmesh.lock(), m_player_navmesh.lock() );
  if ( reg.valid( world_item_entt ) ) reg.destroy( world_item_entt );

  if ( inventory_entity != entt::null ) { m_sound_bank.get_effect( "get_loot" ).play(); }
}

bool PlayerSystem::is_valid_move( const sf::FloatRect &target_position )
{
  auto search_bounds = Cmp::RectBounds::scaled( target_position.position, target_position.size, 1 );

  // Use the spatial hash to check only the ~5 adjacent cells instead of scanning all
  // PlayerNoPath entities (walls + obstacles). Without this, the open spawn area causes
  // a full O(N) scan every frame because no entity triggers an early exit.
  if ( auto navmesh = m_player_navmesh.lock() )
  {
    Cmp::Position target_pos( target_position.position, target_position.size );
    for ( auto candidate_entt : navmesh->neighbours( target_pos, PathFinding::QueryCompass::BOTH ) )
    {
      auto *nopath_cmp = reg().try_get<Cmp::PlayerNoPath>( candidate_entt );
      if ( not nopath_cmp || not nopath_cmp->active ) continue;
      auto *pos_cmp = reg().try_get<Cmp::Position>( candidate_entt );
      if ( not pos_cmp ) continue;
      if ( search_bounds.findIntersection( *pos_cmp ) ) return false;
    }
    return true;
  }

  auto is_active = []( const Cmp::PlayerNoPath &playernopath ) { return playernopath.active; };
  return not Utils::Collision::check_cmp<Cmp::PlayerNoPath>( reg(), search_bounds, is_active );
}

void PlayerSystem::on_drop_inventory_event( Game::Events::DropInventoryEvent ev )
{
  drop_inventory_slot_into_world( ev.drop_pos, ev.inventory_slot_entt );
}

void PlayerSystem::on_player_action_event( Game::Events::PlayerActionEvent ev )
{

  if ( ev.action == Events::PlayerActionEvent::GameActions::DROP_CARRYITEM )
  {
    if ( m_inventory_cooldown_timer.getElapsedTime() < sf::milliseconds( 750.f ) ) return;

    auto player_pos = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ), 0.5 );
    Sprites::SpriteMetaType existing_player_inventory_type;

    // drop inventory if we have one
    auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
    for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
    {
      existing_player_inventory_type = inventory_cmp.m_item.sprite_type;
      drop_inventory_slot_into_world( Utils::Player::get_position( reg() ).position, inventory_entt );
    }

    // pickup inventory if there is something at this position
    auto world_carryitem_view = reg().view<Cmp::WorldItem, Cmp::Position>();
    for ( auto [carryitem_entt, carryitem_cmp, pos_cmp] : world_carryitem_view.each() )
    {
      if ( not player_pos.findIntersection( pos_cmp ) ) continue;                  // is there something to pick up?
      if ( carryitem_cmp.sprite_type == existing_player_inventory_type ) continue; // dont pick up the one we just dropped
      if ( inventory_view.size() > 0 ) { break; }                                  // don't pickup another if we already have one

      // ok pick it up
      pickup_world_item( reg(), carryitem_entt );
    }
    m_inventory_cooldown_timer.restart();
    SPDLOG_DEBUG( "inventory_view: {} ", inventory_view.size() );
  }
  else if ( ev.action == Game::Events::PlayerActionEvent::GameActions::ATTACK )
  {
    // axe attack?!
    check_player_axe_npc_kill();
  }
  else if ( ev.action == Game::Events::PlayerActionEvent::GameActions::DRAW_BOW )
  {
    // draw the bow
    auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
    if ( inventory_slot_type != "sprite.item.bow" ) { return; }
    if ( m_sound_bank.get_effect( "draw_bow" ).getStatus() != sf::Sound::Status::Playing ) m_sound_bank.get_effect( "draw_bow" ).play();
    m_bow_drawing = true;
    m_bow_draw_clock.restart();
  }
  else if ( ev.action == Game::Events::PlayerActionEvent::GameActions::RELEASE_BOW )
  {
    // release an arrow; how far it flies depends on how long the bow was held drawn
    auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
    if ( inventory_slot_type != "sprite.item.bow" ) { return; }

    m_sound_bank.get_effect( "release_bow" ).play();

    static const float kMinChargeFraction = 0.25f;
    static const float kFullDrawSeconds = 1.f;
    float charge_fraction = m_bow_drawing ? std::clamp( m_bow_draw_clock.getElapsedTime().asSeconds() / kFullDrawSeconds, kMinChargeFraction, 1.f )
                                          : 1.f;
    m_bow_drawing = false;

    check_player_fire_arrow( charge_fraction );
  }
  else if ( ev.action == Game::Events::PlayerActionEvent::GameActions::DIG )
  {
    if ( reg().valid( ev.m_entt ) ) pickup_world_item( reg(), ev.m_entt );
  }
}

void PlayerSystem::update_player_no_path_cmp( sf::Time dt )
{
  // PlayerNoPath lives on the plant segment entities (see update_segments), not the
  // PlantMultiBlock entity. Run this before the dig-cooldown early-return, otherwise
  // reactivation stalls in the cooldown window right after digging/replanting.
  static constexpr float kPlantCheckIntervalHz = 2.0f;
  m_plantcheck_accumulator += dt;
  if ( m_plantcheck_accumulator.asSeconds() >= 1.f / kPlantCheckIntervalHz )
  {
    // check if plant player path blocking should be activated
    auto player_pos = Utils::Player::get_position( reg() );
    for ( auto [seg_entt, seg_cmp, playernopath_cmp, seg_pos_cmp] : reg().view<Cmp::PlantSegment, Cmp::PlayerNoPath, Cmp::Position>().each() )
    {
      if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), seg_pos_cmp ) ) continue;

      // enable inactive pathblocking on the segment once the player has moved off its bbox
      if ( playernopath_cmp.active ) continue;
      if ( not player_pos.findIntersection( seg_pos_cmp ) ) { playernopath_cmp.active = true; }
    }
    m_plantcheck_accumulator = sf::Time::Zero;
  }
}

void PlayerSystem::on_player_mortality_event( Game::Events::PlayerMortalityEvent ev )
{

  auto common_death_throes = [&]()
  {
    m_post_death_timer.restart();
    Utils::Player::get_zorder( reg() ).setZOrder( -100 ); // hide the player under the game during animation
    stop_footsteps_sound();
    Utils::Player::get_player_stats( reg() ).apply_modifiers( { Cmp::Stats::Health{ -100 }, {}, {}, {}, {}, {} } );
    SPDLOG_INFO( "Player death code: {}", static_cast<uint8_t>( ev.m_new_state ) );
    Utils::Player::get_mortality( reg() ).state = Cmp::PlayerMortality::State::DEAD;
    SPDLOG_INFO( "Player died" );
  };

  switch ( ev.m_new_state )
  {
    case Cmp::PlayerMortality::State::ALIVE:
      break;

    case Cmp::PlayerMortality::State::FALLING: {
      SPDLOG_INFO( "Player is falling" );
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DECAYING: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::HAUNTED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::EXPLODING: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DROWNING: {
      break;
    }
    case Cmp::PlayerMortality::State::SQUISHED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SUICIDE: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::IGNITED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.flames" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SKEWERED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::SHOCKED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::TERRIFIED: {
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
    }
    case Cmp::PlayerMortality::State::DEAD: {
      break;
    }
    case Cmp::PlayerMortality::State::SHADOWCURSED:
      const auto &sprite = m_sprite_factory.get_spritesheet_by_type( "sprite.death.anim.bloodsplat" );
      Factory::Player::create_player_death_anim( reg(), ev.m_death_pos, sprite );
      m_sound_bank.get_effect( "player_blood_splat" ).play();
      common_death_throes();
      break;
  }
}

} // namespace Game::Sys