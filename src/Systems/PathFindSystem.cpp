#include <Components/AltarSegment.hpp>
#include <Components/Armed.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

PathFindSystem::PathFindSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "PathFindSystem initialized" );
}

void PathFindSystem::update_player_distances()
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

void PathFindSystem::findPath( entt::entity player_entity )
{
  for ( auto [npc_entity, npc_cmp] : getReg().view<Cmp::NPC>().each() )
  {
    scanForPlayers( npc_entity, player_entity );
  }
}

void PathFindSystem::scanForPlayers( entt::entity npc_entity, entt::entity player_entity )
{

  auto npc_scan_bounds = getReg().try_get<Cmp::NPCScanBounds>( npc_entity );
  auto pc_detection_bounds = getReg().try_get<Cmp::PCDetectionBounds>( player_entity );
  if ( not npc_scan_bounds || not pc_detection_bounds ) return;

  // only continue if player is within detection distance
  if ( not npc_scan_bounds->findIntersection( pc_detection_bounds->getBounds() ) ) return;

  // gather up any PlayerDistance components from within range obstacles
  PlayerDistanceQueue distance_queue;
  auto pd_view = getReg().view<Cmp::Position, Cmp::PlayerDistance>( entt::exclude<Cmp::NPC, Cmp::PlayableCharacter> );
  for ( auto [entity, pos_cmp, pd_cmp] : pd_view.each() )
  {
    if ( npc_scan_bounds->findIntersection( pos_cmp ) ) { distance_queue.push( { pd_cmp.distance, entity } ); }
  }

  if ( distance_queue.empty() ) return;
  while ( not distance_queue.empty() )
  {
    // priority queue auto-sorts with the nearest PlayerDistance component at the top
    auto nearest_obstacle = distance_queue.top();
    distance_queue.pop(); // Pop immediately to prevent infinite loop

    // Set the Direction vector and LerpPosition target Coords for next movement towards the player
    auto npc_cmp = getReg().try_get<Cmp::NPC>( npc_entity );
    auto npc_pos = getReg().try_get<Cmp::Position>( npc_entity );
    auto npc_lerp_pos_cmp = getReg().try_get<Cmp::LerpPosition>( npc_entity );
    auto npc_anim_cmp = getReg().try_get<Cmp::SpriteAnimation>( npc_entity );
    if ( not npc_cmp || not npc_pos || not npc_anim_cmp ) return;
    if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) return;

    auto move_candidate_pixel_pos = getPixelPosition( nearest_obstacle.second );
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
      auto npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
      auto candidate_lerp_pos = Cmp::LerpPosition( move_candidate_pixel_pos.value(), npc_lerp_speed.get_value() );

      if ( npc_anim_cmp->m_sprite_type.contains( "NPCGHOST" ) )
      {
        // Ghosts can phase through obstacles, so no need to check for diagonal collisions
        add_candidate_lerp( npc_entity, std::move( candidate_dir ), std::move( candidate_lerp_pos ) );
        return;
      }

      auto horizontal_hitbox = Cmp::RectBounds( sf::Vector2f{ npc_pos->position.x + ( candidate_dir.x * 16 ), npc_pos->position.y },
                                                kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );

      auto vertical_hitbox = Cmp::RectBounds( sf::Vector2f{ npc_pos->position.x, npc_pos->position.y + ( candidate_dir.y * 16 ) },
                                              kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );
      SPDLOG_DEBUG( "Checking distance {} - NPC at ({}, {}), Target at ({}, {}), Dir ({}, {})", nearest_obstacle.first, npc_pos->position.x,
                    npc_pos->position.y, move_candidate_pixel_pos.value().x, move_candidate_pixel_pos.value().y, candidate_dir.x, candidate_dir.y );
      SPDLOG_DEBUG( "Horizontal hitbox at ({}, {}), Vertical hitbox at ({}, {})", npc_pos->position.x + ( candidate_dir.x * 16 ), npc_pos->position.y,
                    npc_pos->position.x, npc_pos->position.y + ( candidate_dir.y * 16 ) );

      bool horizontal_collision = false;
      bool vertical_collision = false;
      auto obst_view = getReg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::PlayerDistance> );
      for ( auto [obst_entity, obst_cmp, obst_pos] : obst_view.each() )
      {
        if ( not pc_detection_bounds->findIntersection( obst_pos ) ) continue;
        if ( not obst_cmp.m_enabled ) continue;
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
      return;
    }
  }
}

void PathFindSystem::add_candidate_lerp( entt::entity npc_entity, Cmp::Direction candidate_dir, Cmp::LerpPosition candidate_lerp_pos )
{
  getReg().emplace_or_replace<Cmp::Direction>( npc_entity, std::move( candidate_dir ) );
  getReg().emplace_or_replace<Cmp::LerpPosition>( npc_entity, std::move( candidate_lerp_pos ) );
}

} // namespace ProceduralMaze::Sys