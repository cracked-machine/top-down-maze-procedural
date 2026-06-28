#include <Components/Moveable.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>

#include <spdlog/spdlog.h>

namespace Game::Sys::ProcGen
{

void CellAutomataSystem::iterate( uint16_t iterations, uint8_t birth_threshold, uint8_t survival_threshold, LevelGenerator::SceneType scene_type,
                                  PathFinding::SpatialHashGrid &levelgen_spatialgrid )
{

  sf::Clock iteration_timer;
  for ( unsigned int i = 0; i < iterations; i++ )
  {

    SPDLOG_DEBUG( "NAVMESH: {}", levelgen_spatialgrid.size() );
    for ( auto [pos_entt, pos_cmp] : reg().view<Cmp::Position>( entt::exclude<Cmp::ReservedPosition> ).each() )
    {
      if ( reg().any_of<Cmp::ReservedPosition>( pos_entt ) ) continue;
      std::vector<entt::entity> neighbour_list = levelgen_spatialgrid.neighbours( pos_cmp );
      SPDLOG_DEBUG( "#{} at {},{} has {} nieghbours", static_cast<uint32_t>( pos_entt ), pos_cmp.x(), pos_cmp.y(), neighbour_list.size() );

      // 1. If the cell is dead and has at least birth-threshold alive neighbors, it becomes alive.
      // 2. If the cell is alive and has at least survival-threshold alive neighbors, it stays alive.
      // 3. Otherwise, the cell is dead.
      const bool is_alive = reg().any_of<Cmp::Obstacle>( pos_entt );
      const bool should_be_alive = ( not is_alive && neighbour_list.size() >= birth_threshold ) ||
                                   ( is_alive && neighbour_list.size() >= survival_threshold );

      if ( should_be_alive )
      {
        if ( scene_type == LevelGenerator::SceneType::GRAVEYARD_EXTERIOR ) { Factory::add_obstacle( reg(), pos_entt ); }
        else if ( scene_type == LevelGenerator::SceneType::RUIN_INTERIOR )
        {
          Factory::add_obstacle( reg(), pos_entt );
          reg().emplace_or_replace<Cmp::Moveable>( pos_entt );
        }
      }
      else
      {
        // make sure we dont delete the player character by accident
        if ( not reg().any_of<Cmp::PlayerCharacter>( pos_entt ) ) { Factory::remove_obstacle( reg(), pos_entt ); }
      }
    }
    SPDLOG_INFO( "Iteration #{} took {}ms", i, iteration_timer.restart().asMilliseconds() );
  }

  SPDLOG_DEBUG( "Total Iterations: {}", iterations );
}

} // namespace Game::Sys::ProcGen