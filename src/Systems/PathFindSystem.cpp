#include <PathFindSystem.hpp>

namespace ProceduralMaze::Sys {

PathFindSystem::PathFindSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
}

void PathFindSystem::findPath( entt::entity player_entity )
{
  for ( auto [npc_entity, npc_cmp] : m_reg->view<Cmp::NPC>().each() )
  {
    scanForPlayers( npc_entity, player_entity );
  }
}

void PathFindSystem::scanForPlayers( entt::entity npc_entity, entt::entity player_entity )
{
  auto npc_scan_bounds = m_reg->try_get<Cmp::NPCScanBounds>( npc_entity );
  auto pc_detection_bounds = m_reg->try_get<Cmp::PCDetectionBounds>( player_entity );
  if ( not npc_scan_bounds || not pc_detection_bounds ) return;

  // only continue if player is within detection distance
  if ( not npc_scan_bounds->findIntersection( pc_detection_bounds->getBounds() ) )
  {
    // remove any out-of-range PlayerDistance components to maintain a small
    // search zone
    m_reg->remove<Cmp::EnttDistanceMap>( npc_entity );
  }
  else
  {
    // gather up any PlayerDistance components from within range obstacles
    PlayerDistanceQueue distance_queue;
    auto obstacle_view = m_reg->view<Cmp::Position, Cmp::PlayerDistance>(
        entt::exclude<Cmp::NPC, Cmp::PlayableCharacter> );
    for ( auto [obstacle_entity, next_pos, player_distance] : obstacle_view.each() )
    {
      if ( npc_scan_bounds->findIntersection( get_hitbox( next_pos ) ) )
      {
        distance_queue.push( { player_distance.distance, obstacle_entity } );
      }
    }

    // Our priority queue auto-sorts with the nearest PlayerDistance component
    // at the top
    if ( distance_queue.empty() ) return;
    auto nearest_obstacle = distance_queue.top();

    // now lets consider moving our NPC. We use lerp to get a smooth
    // transition from grid movements.
    auto npc_cmp = m_reg->try_get<Cmp::NPC>( npc_entity );
    if ( npc_cmp )
    {

      // do not interrupt mid lerp
      auto npc_lerp_pos_cmp = m_reg->try_get<Cmp::LerpPosition>( npc_entity );
      if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) {}
      else
      {
        auto npc_pos = m_reg->try_get<Cmp::Position>( npc_entity );
        auto player_pos = m_reg->try_get<Cmp::Position>( player_entity );
        if ( !npc_pos || !player_pos ) return;
        auto distance = *player_pos - *npc_pos;
        if ( distance != sf::Vector2f( 0.0f, 0.0f ) )
        {
          m_reg->emplace_or_replace<Cmp::Direction>( npc_entity, distance.normalized() );
        }
        // Otherwise set target position for lerp. This will get updated in
        // the main engine loop via LerpSystems
        auto move_candidate_pixel_pos = getPixelPosition( nearest_obstacle.second );
        if ( not move_candidate_pixel_pos ) return;
        auto npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
        m_reg->emplace_or_replace<Cmp::LerpPosition>( npc_entity, move_candidate_pixel_pos.value(),
                                                      npc_lerp_speed.get_value() );
      }
    }
  }
}

} // namespace ProceduralMaze::Sys