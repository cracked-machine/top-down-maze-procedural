#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/Direction.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <CorruptionCell.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <NPCScanBounds.hpp>
#include <NpcSystem.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <SinkholeCell.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <cassert>
#include <cmath>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#define assertm( exp, msg ) assert( ( void( msg ), exp ) )

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem
{
public:
  CollisionSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory );

  ~CollisionSystem() = default;

  // Pause timers when game is paused
  void suspend();

  // Resume timers when game is unpaused
  void resume();

  // Check for player collision with bones obstacles to reanimate NPCs
  void check_bones_reanimation();

  // Check for player collision with NPCs
  void check_player_to_npc_collision();

  // Check for player collision with loot items i.e. player picks up loot
  void check_loot_collision();

  // Check for player collision with obstacles. This is the main collision detection function
  // void check_player_obstacle_collision();

  template <typename HazardType> void check_player_hazard_field_collision()
  {
    auto hazard_field_view = [this]() {
      if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> ) { return m_reg->view<Cmp::SinkholeCell, Cmp::Position>(); }
      else if constexpr ( std::is_same_v<HazardType, Cmp::CorruptionCell> )
      {
        return m_reg->view<Cmp::CorruptionCell, Cmp::Position>();
      }
      else { return m_reg->view<HazardType>(); }
    }();

    auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();

    for ( auto [pc_entt, player_cmp, player_pos_cmp] : player_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getGameView(), player_pos_cmp ) ) continue;

      // make a copy and reduce the player hitbox size to avoid unfair deaths
      auto offset = kGridSquareSizePixelsF / 4.f;
      auto player_hitbox = sf::FloatRect( player_pos_cmp.position + offset, offset * 1.5f );

      for ( auto [hazard_field_entt, hazard_field_cmp, hazard_field_pos_cmp] : hazard_field_view.each() )
      {
        if ( player_hitbox.findIntersection( hazard_field_pos_cmp ) )
        {
          // Player falls into a sinkhole or gets damaged by corruption
          if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> ) { player_cmp.alive = false; }
          else { player_cmp.health -= get_persistent_component<Cmp::Persistent::CorruptionDamage>().get_value(); }
          SPDLOG_DEBUG( "Player fell into a hazard field at position ({}, {})!", hazard_field_pos_cmp.x, hazard_field_pos_cmp.y );
          return; // No need to check further if the player is already dead
        }
      }
    }
  }

  template <typename HazardType> void check_npc_hazard_field_collision()
  {
    auto hazard_field_view = [this]() {
      if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> ) { return m_reg->view<Cmp::SinkholeCell, Cmp::Position>(); }
      else if constexpr ( std::is_same_v<HazardType, Cmp::CorruptionCell> )
      {
        return m_reg->view<Cmp::CorruptionCell, Cmp::Position>();
      }
      else { return m_reg->view<HazardType>(); }
    }();

    auto npc_view = m_reg->view<Cmp::NPC, Cmp::Position>();

    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : npc_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) ) continue;

      for ( auto [hazard_field_entt, hazard_field_cmp, hazard_field_pos_cmp] : hazard_field_view.each() )
      {
        if ( npc_pos_cmp.findIntersection( hazard_field_pos_cmp ) )
        {
          // NPC falls into the sinkhole
          getEventDispatcher().trigger( Events::NpcDeathEvent( npc_entt ) );
          SPDLOG_DEBUG( "NPC fell into a sinkhole at position ({}, {})!", sinkhole_pos_cmp.x, sinkhole_pos_cmp.y );
          return; // No need to check further if the NPC is already dead
        }
      }
    }
  }

  void check_player_large_obstacle_collision( Events::PlayerActionEvent::GameActions action );

  /// EVENT SINKS ///
  void on_player_action( const Events::PlayerActionEvent &event )
  {
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE )
    {
      check_player_large_obstacle_collision( event.action );
    }
  }

private:
  sf::Vector2f findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos,
                                          const sf::Vector2f &player_direction, float pushback_distance );
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__