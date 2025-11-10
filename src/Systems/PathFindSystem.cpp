#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

PathFindSystem::PathFindSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                                Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "PathFindSystem initialized" );
}

void PathFindSystem::update_player_distances()
{
  // precompute outside of loops for performance
  const auto viewBounds = BaseSystem::calculate_view_bounds( RenderSystem::getGameView() );

  // we only have one player so this is just for convenience
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( [[maybe_unused]] auto [pc_entt, pc_cmp, pc_pos_cmp, pc_db_cmp] : player_view.each() )
  {

    // Exclude any components that we dont want NPCs to pathfind through
    auto path_exclusions = entt::exclude<Cmp::ShrineSprite, Cmp::SpawnAreaSprite, Cmp::GraveSprite, Cmp::LootContainer, Cmp::NPC>;

    auto path_view = m_reg->view<Cmp::Position>( path_exclusions );
    for ( auto [path_entt, path_pos_cmp] : path_view.each() )
    {
      // optimization
      if ( !is_visible_in_view( viewBounds, path_pos_cmp ) ) continue;

      // we can't filter out obstacles in the view, we have to check its enabled bit
      auto obst_cmp = m_reg->try_get<Cmp::Obstacle>( path_entt );
      if ( obst_cmp && obst_cmp->m_enabled ) continue;

      // calculate the distance from the position to the player
      if ( pc_db_cmp.findIntersection( path_pos_cmp ) )
      {
        auto distance = std::floor( getEuclideanDistance( pc_pos_cmp.position, path_pos_cmp.position ) );
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
  if ( npc_scan_bounds->findIntersection( pc_detection_bounds->getBounds() ) )
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
    SPDLOG_DEBUG( " NPC entity {} found {} obstacles within scan bounds", static_cast<int>( npc_entity ), distance_queue.size() );

    auto nearest_obstacle = distance_queue.top();

    // Set the Direction vector and LerpPosition target Coords for next movement towards the player
    auto npc_cmp = m_reg->try_get<Cmp::NPC>( npc_entity );
    if ( not npc_cmp ) return;

    auto npc_lerp_pos_cmp = m_reg->try_get<Cmp::LerpPosition>( npc_entity );
    if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) return;

    auto npc_pos = m_reg->try_get<Cmp::Position>( npc_entity );
    auto player_pos = m_reg->try_get<Cmp::Position>( player_entity );
    if ( !npc_pos || !player_pos ) return;

    auto distance = ( *player_pos ).position - ( *npc_pos ).position;
    SPDLOG_INFO( " NPC entity {} moving towards player from obstacle at distance {},{}", static_cast<int>( npc_entity ), distance.x,
                 distance.y );
    if ( distance == sf::Vector2f( 0.0f, 0.0f ) ) return;

    m_reg->emplace_or_replace<Cmp::Direction>( npc_entity, distance.normalized() );

    auto move_candidate_pixel_pos = getPixelPosition( nearest_obstacle.second );
    if ( not move_candidate_pixel_pos ) return;

    auto npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
    m_reg->emplace_or_replace<Cmp::LerpPosition>( npc_entity, move_candidate_pixel_pos.value(), npc_lerp_speed.get_value() );
  }
}

} // namespace ProceduralMaze::Sys