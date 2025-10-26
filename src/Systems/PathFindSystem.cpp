#include <FootStepAlpha.hpp>
#include <FootStepTimer.hpp>
#include <GraveSprite.hpp>
#include <LootContainer.hpp>
#include <PathFindSystem.hpp>
#include <RenderSystem.hpp>
#include <ShrineSprite.hpp>
#include <SpawnAreaSprite.hpp>

namespace ProceduralMaze::Sys {

PathFindSystem::PathFindSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
}

void PathFindSystem::update_player_distances()
{
  // precompute outside of loops for performance
  const auto viewBounds = BaseSystem::calculate_view_bounds( RenderSystem::getGameView() );

  // we only have one player so this is just for convenience
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( [[maybe_unused]] auto [pc_entt, pc_cmp, pc_pos_cmp, pc_db_cmp] : player_view.each() )
  {

    // clang-format off
    // Exclude any components that we dont want NPCs to pathfind through
    auto path_exclusions = entt::exclude<
      Cmp::ShrineSprite, Cmp::SpawnAreaSprite, 
      Cmp::GraveSprite,
      Cmp::LootContainer
    >;
    // clang-format on
    auto path_view = m_reg->view<Cmp::Position>( path_exclusions );
    for ( auto [path_entt, path_pos_cmp] : path_view.each() )
    {
      if ( !is_visible_in_view( viewBounds, path_pos_cmp ) ) continue;
      // we can't filter out obstacles in the view, we have to check its enabled bit
      auto obst_cmp = m_reg->try_get<Cmp::Obstacle>( path_entt );
      if ( obst_cmp && obst_cmp->m_enabled ) continue;

      // calculate the distance from the position to the player
      if ( pc_db_cmp.findIntersection( path_pos_cmp ) )
      {
        auto distance = std::floor( getChebyshevDistance( pc_pos_cmp.position, path_pos_cmp.position ) );
        m_reg->emplace_or_replace<Cmp::PlayerDistance>( path_entt, distance );
      }
      else
      {
        // tidy up any out of range obstacles
        m_reg->remove<Cmp::PlayerDistance>( path_entt );
      }
    }
  }
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
    // remove any out-of-range EnttDistanceMap components to maintain a small search zone
    m_reg->remove<Cmp::EnttDistanceMap>( npc_entity );
  }
  else
  {
    // gather up any PlayerDistance components from within range obstacles
    PlayerDistanceQueue distance_queue;
    auto pd_view = m_reg->view<Cmp::Position, Cmp::PlayerDistance>( entt::exclude<Cmp::NPC, Cmp::PlayableCharacter> );
    for ( auto [entity, pos_cmp, pd_cmp] : pd_view.each() )
    {
      if ( npc_scan_bounds->findIntersection( pos_cmp ) ) { distance_queue.push( { pd_cmp.distance, entity } ); }
    }

    // Our priority queue auto-sorts with the nearest PlayerDistance component at the top
    if ( distance_queue.empty() ) return;
    SPDLOG_DEBUG( " NPC entity {} found {} obstacles within scan bounds", static_cast<int>( npc_entity ),
                  distance_queue.size() );

    auto nearest_obstacle = distance_queue.top();

    // now lets consider moving our NPC. We use lerp to get a smooth transition from grid movements.
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
        auto distance = ( *player_pos ).position - ( *npc_pos ).position;
        if ( distance != sf::Vector2f( 0.0f, 0.0f ) )
        {
          m_reg->emplace_or_replace<Cmp::Direction>( npc_entity, distance.normalized() );
        }
        // Otherwise set target position for lerp. This will get updated in the main engine loop via LerpSystems
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