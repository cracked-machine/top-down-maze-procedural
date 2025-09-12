#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <Components/EnttDistanceMap.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/NpcLerpSpeed.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Systems/BaseSystem.hpp>

#include <cstdlib>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <array>
#include <cmath>

#include <queue>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

using PlayerDistanceQueue =
    std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>, std::greater<std::pair<int, entt::entity>>>;

class PathFindSystem : public BaseSystem
{
public:
  PathFindSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : BaseSystem( reg ) {}
  ~PathFindSystem() = default;

  void init_context()
  {
    if ( not m_reg->ctx().contains<Cmp::Persistent::NpcLerpSpeed>() ) { m_reg->ctx().emplace<Cmp::Persistent::NpcLerpSpeed>(); }
  }

  void findPath( entt::entity player_entity )
  {
    for ( auto [npc_entity, npc_cmp] : m_reg->view<Cmp::NPC>().each() )
    {
      scanForPlayers( npc_entity, player_entity );
    }
  }

  void scanForPlayers( entt::entity npc_entity, entt::entity player_entity )
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
      auto obstacle_view = m_reg->view<Cmp::Position, Cmp::PlayerDistance>( entt::exclude<Cmp::NPC, Cmp::PlayableCharacter> );
      for ( auto [obstacle_entity, next_pos, player_distance] : obstacle_view.each() )
      {
        if ( npc_scan_bounds->findIntersection( get_hitbox( next_pos ) ) ) { distance_queue.push( { player_distance.distance, obstacle_entity } ); }
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
          m_reg->emplace_or_replace<Cmp::Direction>( npc_entity, distance.normalized() );

          // Otherwise set target position for lerp. This will get updated in
          // the main engine loop via LerpSystems
          auto move_candidate_pixel_pos = getPixelPosition( nearest_obstacle.second );
          if ( not move_candidate_pixel_pos ) return;
          auto npc_lerp_speed = m_reg->ctx().get<Cmp::Persistent::NpcLerpSpeed>();
          m_reg->emplace_or_replace<Cmp::LerpPosition>( npc_entity, move_candidate_pixel_pos.value(), 0.0f, npc_lerp_speed() );
        }
      }
    }
  }

private:
  // Define possible movement directions (up, right, down, left)
  const std::array<sf::Vector2f, 4> m_directions = {
      sf::Vector2f( 0.f, -Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y ), // Up
      sf::Vector2f( Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.x, 0.f ),  // Right
      sf::Vector2f( 0.f, Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y ),  // Down
      sf::Vector2f( -Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.x, 0.f )  // Left
  };

  // the limit for finding potential paths
  // const int SCAN_DISTANCE{1};
  // the activation distance for NPCs
  // const unsigned int AGGRO_DISTANCE{5};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__