#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcContainer.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Npc/NpcScanBounds.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerDetectionBounds.hpp>
#include <Components/Player/PlayerDistance.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <queue>

namespace ProceduralMaze::Sys
{

using PlayerDistanceQueue = std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>,
                                                std::greater<std::pair<int, entt::entity>>>;

NpcSystem::NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "NpcSystem initialized" );
}

void NpcSystem::update( sf::Time dt )
{
  check_bones_reanimation();

  auto player_entity_view = getReg().view<Cmp::PlayerCharacter>();
  for ( auto player_entity : player_entity_view )
  {
    scanForPlayers( player_entity );
  }

  update_movement( dt );

  for ( auto [_ent, _sys] : getReg().view<Cmp::System>().each() )
  {
    if ( _sys.collisions_enabled ) { check_player_to_npc_collision(); }
  }

  for ( auto npc_entt : getReg().view<Cmp::NPC>() )
  {
    auto npc_sprite_anim = getReg().try_get<Cmp::SpriteAnimation>( npc_entt );
    if ( npc_sprite_anim && npc_sprite_anim->m_sprite_type == "NPCPRIEST" ) { emit_shockwave( npc_entt ); }
  }
  update_shockwaves();
}

bool NpcSystem::is_valid_move( const sf::FloatRect &target_position )
{

  // Prevent the player from walking through NPCs
  auto &pc_damage_delay = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PcDamageDelay>( getReg() );
  auto npc_view = getReg().view<Cmp::NPC, Cmp::Position, Cmp::LerpPosition>();
  auto pc_view = getReg().view<Cmp::PlayerCharacter>();
  for ( auto [pc_entity, pc_cmp] : pc_view.each() )
  {
    // However if player is in damage cooldown (blinking), let player walk through NPCs to escape
    if ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_delay.get_value() ) continue;
    for ( auto [entity, npc_cmp, pos_cmp, lerp_pos_cmp] : npc_view.each() )
    {
      // relaxed bounds to allow player to sneak past during lerp transition
      Cmp::RectBounds npc_pos_cmp_bounds_current{ pos_cmp.position, pos_cmp.size, 0.1f };
      if ( target_position.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) { return false; }
    }
  }
  // Check obstacles
  auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obs_cmp, pos_cmp] : obstacle_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided Obstacle" );
      return false;
    }
  }

  // Check walls
  auto wall_view = getReg().view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      if ( getReg().any_of<Cmp::NpcNoPathFinding>( entity ) )
      {
        SPDLOG_DEBUG( "Player Collided Wall" );
        return false;
      }
    }
  }

  // arbitrary Cmp::NoPathFinding components
  auto nppathfinding_view = getReg().view<Cmp::NpcNoPathFinding, Cmp::Position>();
  for ( auto [entt, nopath_cmp, pos_cmp] : nppathfinding_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided Cmp::NoPathFinding" );
      return false;
    }
  }

  // Check doors
  auto door_view = getReg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, pos_cmp] : door_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      if ( exit_cmp.m_locked == false ) { return true; }
      else { return false; }
    }
  }

  auto grave_view = getReg().view<Cmp::GraveSegment, Cmp::Position>();
  for ( auto [entity, grave_cmp, pos_cmp] : grave_view.each() )
  {
    if ( not grave_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided GraveSegment" );
      return false;
    }
  }

  auto altar_view = getReg().view<Cmp::AltarSegment, Cmp::Position>();
  for ( auto [entity, altar_cmp, pos_cmp] : altar_view.each() )
  {
    if ( not altar_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided AltarSegment" );
      return false;
    }
  }

  auto crypt_view = getReg().view<Cmp::CryptSegment, Cmp::Position>();
  for ( auto [entity, crypt_cmp, pos_cmp] : crypt_view.each() )
  {
    if ( not crypt_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided CryptSegment" );
      return false;
    }
  }

  auto holywell_view = getReg().view<Cmp::HolyWellSegment, Cmp::Position>();
  for ( auto [entity, holywell_cmp, pos_cmp] : holywell_view.each() )
  {
    if ( not holywell_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided HolyWellSegment" );
      return false;
    }
  }

  auto ruin_view = getReg().view<Cmp::RuinSegment, Cmp::Position>();
  for ( auto [entity, ruin_cmp, pos_cmp] : ruin_view.each() )
  {
    if ( not ruin_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided RuinSegment" );
      return false;
    }
  }

  auto ruin_stair_view = getReg().view<Cmp::RuinStairsSegment, Cmp::Position>();
  for ( auto [entity, ruin_cmp, pos_cmp] : ruin_stair_view.each() )
  {
    if ( not ruin_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided RuinStairsSegment" );
      return false;
    }
  }

  auto crypt_obj_view = getReg().view<Cmp::CryptObjectiveSegment, Cmp::Position>();
  for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : crypt_obj_view.each() )
  {
    if ( not crypt_obj_cmp.isSolidMask() ) continue;
    if ( crypt_obj_pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided CryptObjectiveSegment" );
      return false;
    }
  }

  auto crypt_int_view = getReg().view<Cmp::CryptInteriorSegment, Cmp::Position>();
  for ( auto [entity, crypt_int_cmp, crypt_int_pos_cmp] : crypt_int_view.each() )
  {
    if ( not crypt_int_cmp.isSolidMask() ) continue;
    if ( crypt_int_pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided CryptInteriorSegment" );
      return false;
    }
  }

  auto plant_view = getReg().view<Cmp::PlantObstacle, Cmp::Position>();
  for ( auto [entity, plant_cmp, plant_pos_cmp] : plant_view.each() )
  {
    if ( plant_pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Player Collided PlantObstacle " );
      return false;
    }
  }

  auto crypt_chest_view = getReg().view<Cmp::CryptChest, Cmp::Position>();
  for ( auto [entity, crypt_chest_cmp, crypt_chest_pos_cmp] : crypt_chest_view.each() )
  {
    if ( crypt_chest_pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "Blocking player at {},{} with CryptChest", crypt_chest_pos_cmp.position.x, crypt_chest_pos_cmp.position.y );
      return false;
    }
  }

  return true;
}

//! @brief Check if diagonal movement should be blocked due to adjacent obstacles
//! @param current_pos Current position rectangle
//! @param diagonal_direction The diagonal direction vector (e.g., {1, -1} for up-right)
//! @return true if diagonal movement is blocked by adjacent obstacles
bool NpcSystem::isDiagonalBlocked( const sf::FloatRect &current_pos, const sf::Vector2f &diagonal_direction )
{
  // Only check for diagonal movements (both x and y components non-zero)
  if ( diagonal_direction.x == 0.f || diagonal_direction.y == 0.f )
  {
    return false; // Not a diagonal move
  }

  // Create test positions for the two cardinal directions
  sf::FloatRect horizontal_test = current_pos;
  horizontal_test.position.x += diagonal_direction.x * Constants::kGridSquareSizePixelsF.x;

  sf::FloatRect vertical_test = current_pos;
  vertical_test.position.y += diagonal_direction.y * Constants::kGridSquareSizePixelsF.y;

  // If EITHER cardinal direction is blocked, block the diagonal
  bool horizontal_blocked = !is_valid_move( horizontal_test );
  bool vertical_blocked = !is_valid_move( vertical_test );

  return horizontal_blocked || vertical_blocked;
}

void NpcSystem::update_movement( sf::Time globalDeltaTime )
{
  auto exclusions = entt::exclude<Cmp::AltarSegment, Cmp::CryptSegment, Cmp::SpawnArea, Cmp::PlayerCharacter>;
  auto view = getReg().view<Cmp::Position, Cmp::LerpPosition, Cmp::NPCScanBounds, Cmp::Direction>( exclusions );

  for ( auto [entity, pos_cmp, lerp_pos_cmp, npc_scan_bounds, dir_cmp] : view.each() )
  {

    // if there is an obstacle at this entity move onto the next entity
    auto obst_cmp = getReg().try_get<Cmp::Obstacle>( entity );
    if ( obst_cmp ) continue;

    // If this is the first update, store the start position
    if ( lerp_pos_cmp.m_lerp_factor == 0.0f )
    {
      // Allow NPCs to escape wormholes if they're mid-lerp.
      if ( getReg().try_get<Cmp::WormholeJump>( entity ) ) continue;

      lerp_pos_cmp.m_start = pos_cmp.position;
    }

    lerp_pos_cmp.m_lerp_factor += lerp_pos_cmp.m_lerp_speed * globalDeltaTime.asSeconds();

    if ( lerp_pos_cmp.m_lerp_factor >= 1.0f )
    {
      pos_cmp.position = lerp_pos_cmp.m_target;
      getReg().remove<Cmp::LerpPosition>( entity );
    }
    else
    {
      // Lerp from start to target directly
      pos_cmp.position.x = std::lerp( lerp_pos_cmp.m_start.x, lerp_pos_cmp.m_target.x, lerp_pos_cmp.m_lerp_factor );
      pos_cmp.position.y = std::lerp( lerp_pos_cmp.m_start.y, lerp_pos_cmp.m_target.y, lerp_pos_cmp.m_lerp_factor );
    }

    // clang-format off
    getReg().patch<Cmp::NPCScanBounds>( entity, 
    [&]( auto &npc_scan_bounds ) 
    { 
      npc_scan_bounds.position( pos_cmp.position ); 
    });
    getReg().patch<Cmp::ZOrderValue>( entity, 
    [&]( auto &zorder_cmp ) 
    { 
      zorder_cmp.setZOrder( pos_cmp.position.y ); 
    });
    // clang-format on
  }
}

void NpcSystem::check_bones_reanimation()
{
  auto player_collision_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto npccontainer_collision_view = getReg().view<Cmp::NpcContainer, Cmp::Position>();
  for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    for ( auto [npccontainer_entt, npccontainer_cmp, npccontainer_pos_cmp] : npccontainer_collision_view.each() )
    {
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), npccontainer_pos_cmp ) ) continue;

      auto &npc_activate_scale = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcActivateScale>( getReg() );
      // we just create a temporary RectBounds here instead of a component because we only need it
      // for this one comparison and it already contains the needed scaling logic
      auto npc_activate_bounds = Cmp::RectBounds( npccontainer_pos_cmp.position, Constants::kGridSquareSizePixelsF, npc_activate_scale.get_value() );

      if ( pc_pos_cmp.findIntersection( npc_activate_bounds.getBounds() ) )
      {
        Factory::createNPC( getReg(), npccontainer_entt, "NPCSKELE" );
        m_sound_bank.get_effect( "spawn_skeleton" ).play();
      }
    }
  }
}

void NpcSystem::check_player_to_npc_collision()
{
  auto player_collision_view = getReg().view<Cmp::PlayerCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position, Cmp::Direction>();
  auto npc_collision_view = getReg().view<Cmp::NPC, Cmp::Position>();
  auto &npc_push_back = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcPushBack>( getReg() );
  auto &pc_damage_cooldown = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PcDamageDelay>( getReg() );

  for ( auto [pc_entity, pc_cmp, pc_health_cmp, pc_mort_cmp, pc_pos_cmp, dir_cmp] : player_collision_view.each() )
  {
    if ( pc_mort_cmp.state != Cmp::PlayerMortality::State::ALIVE ) return;
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_collision_view.each() )
    {
      // relaxed bounds to allow player to sneak past during lerp transition
      Cmp::RectBounds npc_pos_cmp_bounds_current{ npc_pos_cmp.position, npc_pos_cmp.size, 0.1f };
      if ( not pc_pos_cmp.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) continue;

      if ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() ) continue;

      auto &npc_damage = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcDamage>( getReg() );
      pc_health_cmp.health -= npc_damage.get_value();

      m_sound_bank.get_effect( "damage_player" ).play();

      if ( pc_health_cmp.health <= 0 )
      {
        pc_mort_cmp.state = Cmp::PlayerMortality::State::HAUNTED;
        get_systems_event_queue().enqueue(
            Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::HAUNTED, Utils::get_player_position( getReg() ) ) );
        return;
      }

      pc_cmp.m_damage_cooldown_timer.restart();

      // Find a valid pushback position by checking all 8 directions
      sf::Vector2f target_push_back_pos = findValidPushbackPosition( pc_pos_cmp.position, npc_pos_cmp.position, dir_cmp, npc_push_back.get_value() );

      // Update player position if we found a valid pushback position
      if ( target_push_back_pos != pc_pos_cmp.position ) { pc_pos_cmp.position = target_push_back_pos; }
    }
  }
}

sf::Vector2f NpcSystem::findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos, const sf::Vector2f &player_direction,
                                                   float pushback_distance )
{
  // Define all 8 directions (N, NE, E, SE, S, SW, W, NW)
  std::vector<sf::Vector2f> directions = {
      { 0.0f, -1.0f }, // North
      { 1.0f, -1.0f }, // North-East
      { 1.0f, 0.0f },  // East
      { 1.0f, 1.0f },  // South-East
      { 0.0f, 1.0f },  // South
      { -1.0f, 1.0f }, // South-West
      { -1.0f, 0.0f }, // West
      { -1.0f, -1.0f } // North-West
  };

  // Priority order for direction selection
  std::vector<sf::Vector2f> preferred_directions;

  if ( player_direction != sf::Vector2f( 0.0f, 0.0f ) )
  {
    // Player is moving - prefer pushing opposite to movement direction
    sf::Vector2f opposite_dir = -player_direction;

    // Normalize for comparison
    float mag = std::sqrt( opposite_dir.x * opposite_dir.x + opposite_dir.y * opposite_dir.y );
    if ( mag > 0.0f )
    {
      opposite_dir.x /= mag;
      opposite_dir.y /= mag;
    }

    // Find the closest matching cardinal/diagonal direction
    float best_dot = -2.0f;
    sf::Vector2f best_dir;
    for ( const auto &dir : directions )
    {
      float dot = opposite_dir.x * dir.x + opposite_dir.y * dir.y;
      if ( dot > best_dot )
      {
        best_dot = dot;
        best_dir = dir;
      }
    }

    preferred_directions.push_back( best_dir );

    // Add perpendicular directions as secondary options
    // Find directions that are perpendicular to best_dir
    for ( const auto &dir : directions )
    {
      float dot = std::abs( best_dir.x * dir.x + best_dir.y * dir.y );
      if ( dot < 0.1f ) // approximately perpendicular
      {
        preferred_directions.push_back( dir );
      }
    }
  }
  else
  {
    // Player is stationary - prefer pushing away from NPC
    sf::Vector2f away_from_npc = player_pos - npc_pos;
    if ( away_from_npc != sf::Vector2f( 0.0f, 0.0f ) )
    {
      // Normalize for comparison
      float mag = std::sqrt( away_from_npc.x * away_from_npc.x + away_from_npc.y * away_from_npc.y );
      if ( mag > 0.0f )
      {
        away_from_npc.x /= mag;
        away_from_npc.y /= mag;
      }

      // Find the closest matching cardinal/diagonal direction
      float best_dot = -2.0f;
      sf::Vector2f best_dir;
      for ( const auto &dir : directions )
      {
        float dot = away_from_npc.x * dir.x + away_from_npc.y * dir.y;
        if ( dot > best_dot )
        {
          best_dot = dot;
          best_dir = dir;
        }
      }
      preferred_directions.push_back( best_dir );
    }
  }

  // Add all 8 directions to ensure we check everything
  for ( const auto &dir : directions )
  {
    preferred_directions.push_back( dir );
  }

  // Try each direction in priority order
  for ( const auto &push_dir : preferred_directions )
  {
    sf::FloatRect candidate_pos{ player_pos + push_dir * pushback_distance, Constants::kGridSquareSizePixelsF };
    candidate_pos = Utils::snap_to_grid( candidate_pos );

    // Check if this position is valid and different from current position
    if ( candidate_pos.position != player_pos && is_valid_move( candidate_pos ) ) { return candidate_pos.position; }
  }

  // If no valid position found, return original position
  return player_pos;
}

void NpcSystem::scanForPlayers( entt::entity player_entity )
{
  for ( auto [npc_entity, npc_cmp] : getReg().view<Cmp::NPC>().each() )
  {
    auto npc_scan_bounds = getReg().try_get<Cmp::NPCScanBounds>( npc_entity );
    auto pc_detection_bounds = getReg().try_get<Cmp::PCDetectionBounds>( player_entity );
    if ( not npc_scan_bounds || not pc_detection_bounds ) continue;

    // only continue if player is within detection distance
    // if ( not npc_scan_bounds->findIntersection( pc_detection_bounds->getBounds() ) ) continue;

    // gather up any PlayerDistance components from within range obstacles
    PlayerDistanceQueue distance_queue;
    auto pd_view = getReg().view<Cmp::Position, Cmp::PlayerDistance>( entt::exclude<Cmp::NPC, Cmp::PlayerCharacter> );
    for ( auto [entity, pos_cmp, pd_cmp] : pd_view.each() )
    {
      // footsteps are always tracked, everything else needs to be within scan bounds
      if ( not getReg().all_of<Cmp::FootStepTimer>( entity ) )
      {
        if ( not npc_scan_bounds->findIntersection( pc_detection_bounds->getBounds() ) ) continue;
      }
      if ( npc_scan_bounds->findIntersection( pos_cmp ) ) { distance_queue.push( { pd_cmp.distance, entity } ); }
    }

    if ( distance_queue.empty() ) continue;
    while ( not distance_queue.empty() )
    {
      // priority queue auto-sorts with the nearest PlayerDistance component at the top
      auto nearest_obstacle = distance_queue.top();
      distance_queue.pop(); // Pop immediately to prevent infinite loop

      // Set the Direction vector and LerpPosition target Coords for next movement towards the
      // player
      auto npc_cmp = getReg().try_get<Cmp::NPC>( npc_entity );
      auto npc_pos = getReg().try_get<Cmp::Position>( npc_entity );
      auto npc_lerp_pos_cmp = getReg().try_get<Cmp::LerpPosition>( npc_entity );
      auto npc_anim_cmp = getReg().try_get<Cmp::SpriteAnimation>( npc_entity );
      if ( not npc_cmp || not npc_pos || not npc_anim_cmp ) continue;
      if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) continue; // still mid-lerp, wait until done

      auto move_candidate_pixel_pos = Utils::getPixelPosition( getReg(), nearest_obstacle.second );
      if ( not move_candidate_pixel_pos ) continue; // Try next candidate

      // Calculate direction from NPC to target cell and update animation state
      auto direction_to_target = move_candidate_pixel_pos.value() - npc_pos->position;
      if ( direction_to_target == sf::Vector2f( 0.0f, 0.0f ) )
      {
        npc_anim_cmp->m_animation_active = false;
        continue;
      }
      else
      {
        npc_anim_cmp->m_animation_active = true;
        if ( npc_anim_cmp->m_sprite_type.contains( "NPCSKELE" ) )
        {

          if ( direction_to_target.x > 0 ) { npc_anim_cmp->m_sprite_type = "NPCSKELE.walk.east"; }
          else if ( direction_to_target.x < 0 ) { npc_anim_cmp->m_sprite_type = "NPCSKELE.walk.west"; }
          else if ( direction_to_target.y < 0 ) { npc_anim_cmp->m_sprite_type = "NPCSKELE.walk.north"; }
          else if ( direction_to_target.y > 0 ) { npc_anim_cmp->m_sprite_type = "NPCSKELE.walk.south"; }
        }
        else if ( npc_anim_cmp->m_sprite_type.contains( "NPCGHOST" ) )
        {
          // Ghost NPCs face cardinal directions only
          if ( direction_to_target.x > 0 ) { npc_anim_cmp->m_sprite_type = "NPCGHOST.walk.east"; }
          else if ( direction_to_target.x < 0 ) { npc_anim_cmp->m_sprite_type = "NPCGHOST.walk.west"; }
          else if ( direction_to_target.y < 0 ) { npc_anim_cmp->m_sprite_type = "NPCGHOST.walk.north"; }
          else if ( direction_to_target.y > 0 ) { npc_anim_cmp->m_sprite_type = "NPCGHOST.walk.south"; }
        }

        auto candidate_dir = Cmp::Direction( direction_to_target.normalized() );
        auto npc_lerp_speed = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcLerpSpeed>( getReg() );
        auto candidate_lerp_pos = Cmp::LerpPosition( move_candidate_pixel_pos.value(), npc_lerp_speed.get_value() );

        if ( npc_anim_cmp->m_sprite_type.contains( "NPCGHOST" ) )
        {
          // Ghosts can phase diagonally between obstacles, so no need to check for diagonal
          // collisions
          add_candidate_lerp( npc_entity, std::move( candidate_dir ), std::move( candidate_lerp_pos ) );
          return;
        }

        // Check if this is a diagonal movement and if it should be blocked
        bool is_diagonal = ( candidate_dir.x != 0.0f ) && ( candidate_dir.y != 0.0f );
        if ( is_diagonal && isDiagonalBlocked( sf::FloatRect{ npc_pos->position, npc_pos->size }, candidate_dir ) )
        {
          SPDLOG_DEBUG( "Blocking diagonal movement for NPC at ({}, {}) towards ({}, {})", npc_pos->position.x, npc_pos->position.y,
                        move_candidate_pixel_pos.value().x, move_candidate_pixel_pos.value().y );
          continue; // Skip this target and try the next one
        }

        auto horizontal_hitbox = Cmp::RectBounds( sf::Vector2f{ npc_pos->position.x + ( candidate_dir.x * 16 ), npc_pos->position.y },
                                                  Constants::kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );

        auto vertical_hitbox = Cmp::RectBounds( sf::Vector2f{ npc_pos->position.x, npc_pos->position.y + ( candidate_dir.y * 16 ) },
                                                Constants::kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );
        SPDLOG_DEBUG( "Checking distance {} - NPC at ({}, {}), Target at ({}, {}), Dir ({}, {})", nearest_obstacle.first, npc_pos->position.x,
                      npc_pos->position.y, move_candidate_pixel_pos.value().x, move_candidate_pixel_pos.value().y, candidate_dir.x, candidate_dir.y );
        SPDLOG_DEBUG( "Horizontal hitbox at ({}, {}), Vertical hitbox at ({}, {})", npc_pos->position.x + ( candidate_dir.x * 16 ),
                      npc_pos->position.y, npc_pos->position.x, npc_pos->position.y + ( candidate_dir.y * 16 ) );

        bool horizontal_collision = false;
        bool vertical_collision = false;
        auto obst_view = getReg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::PlayerDistance> );
        for ( auto [obst_entity, obst_cmp, obst_pos] : obst_view.each() )
        {
          if ( not pc_detection_bounds->findIntersection( obst_pos ) ) continue;
          if ( horizontal_hitbox.findIntersection( obst_pos ) )
          {
            SPDLOG_DEBUG( "!!!! Horizontal collision at obstacle ({}, {})", obst_pos.position.x, obst_pos.position.y );
            horizontal_collision = true;
          }
          if ( vertical_hitbox.findIntersection( obst_pos ) )
          {
            SPDLOG_DEBUG( "!!!! Vertical collision at obstacle ({}, {})", obst_pos.position.x, obst_pos.position.y );
            vertical_collision = true;
          }
        }

        if ( horizontal_collision && vertical_collision )
        {

          SPDLOG_DEBUG( "Exclude obstacle at distance {}", nearest_obstacle.first );
          continue;
        }

        // Target is valid
        add_candidate_lerp( npc_entity, std::move( candidate_dir ), std::move( candidate_lerp_pos ) );
        break;
      }
    }
  }
}

void NpcSystem::add_candidate_lerp( entt::entity npc_entity, Cmp::Direction candidate_dir, Cmp::LerpPosition candidate_lerp_pos )
{
  getReg().emplace_or_replace<Cmp::Direction>( npc_entity, std::move( candidate_dir ) );
  getReg().emplace_or_replace<Cmp::LerpPosition>( npc_entity, std::move( candidate_lerp_pos ) );
}

void NpcSystem::emit_shockwave( entt::entity npc_entt )
{
  // cooldown is handled in Factory function via Cmp::NpcShockwaveTimer per NPC
  Factory::createShockwave( getReg(), npc_entt );
}

void NpcSystem::update_shockwaves()
{
  auto shockwave_increments = 1;

  // Invert the interval - larger values = faster updates, smaller values = slower updates
  auto speed_value = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcShockwaveSpeed>( getReg() ).get_value();
  sf::Time shockwave_update_interval{ sf::milliseconds( static_cast<int>( 1000.0f / speed_value ) ) };

  auto max_radius = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::NpcShockwaveMaxRadius>( getReg() );

  if ( shockwave_update_clock.getElapsedTime() > shockwave_update_interval )
  {
    for ( auto entt : getReg().view<Cmp::NpcShockwave>() )
    {
      auto &sw_cmp = getReg().get<Cmp::NpcShockwave>( entt );
      float current_radius = sw_cmp.sprite.getRadius();

      // Exponential scaling - shockwave accelerates as it grows
      // This creates a natural acceleration that maintains the visual impression of constant speed as the circumference grows.
      float normalized_radius = current_radius / max_radius.get_value();
      float speed_multiplier = 1.0f + normalized_radius * normalized_radius; // Quadratic acceleration

      float new_radius = current_radius + ( shockwave_increments * speed_multiplier );
      sw_cmp.sprite.setRadius( new_radius );

      checkShockwaveObstacleCollision( entt, sw_cmp );

      if ( new_radius > max_radius.get_value() ) { getReg().destroy( entt ); }
    }
    shockwave_update_clock.restart();
  }
}

void NpcSystem::checkShockwaveObstacleCollision( [[maybe_unused]] entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave )
{
  auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position, Cmp::SpriteAnimation>();

  for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos, sprite_anim] : obstacle_view.each() )
  {
    // Check if this is the specific obstacle type and index we care about
    if ( ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 3 ) or
         ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 2 ) or
         ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 0 ) )
    {
      sf::FloatRect obstacle_rect( obstacle_pos.position, obstacle_pos.size );

      SPDLOG_DEBUG( "Checking obstacle at ({}, {}) size ({}, {}) against shockwave at ({}, {}) radius {}", obstacle_rect.position.x,
                    obstacle_rect.position.y, obstacle_rect.size.x, obstacle_rect.size.y, shockwave.getPosition().x, shockwave.getPosition().y,
                    shockwave.getRadius() );

      // Calculate distance from circle center to rectangle
      sf::Vector2f circle_center = shockwave.sprite.getPosition();
      float circle_radius = shockwave.sprite.getRadius();

      // Find closest point on rectangle to circle center
      sf::Vector2f closest_point;
      closest_point.x = std::max( obstacle_rect.position.x, std::min( circle_center.x, obstacle_rect.position.x + obstacle_rect.size.x ) );
      closest_point.y = std::max( obstacle_rect.position.y, std::min( circle_center.y, obstacle_rect.position.y + obstacle_rect.size.y ) );

      // Calculate distance from circle center to closest point
      sf::Vector2f diff = circle_center - closest_point;
      float distance = std::sqrt( diff.x * diff.x + diff.y * diff.y );

      // Check if circle intersects with rectangle (accounting for outline thickness)
      float effective_radius = circle_radius + shockwave.sprite.getOutlineThickness() / 2.0f;

      if ( distance <= effective_radius )
      {
        SPDLOG_DEBUG( "Shockwave INTERSECTS with obstacle (distance: {}, effective_radius: {})", distance, effective_radius );
        Sys::ShockwaveSystem::removeIntersectingSegments( obstacle_rect, shockwave );
      }
      else { SPDLOG_DEBUG( "Shockwave does NOT intersect with obstacle (distance: {}, effective_radius: {})", distance, effective_radius ); }
    }
  }
}

} // namespace ProceduralMaze::Sys