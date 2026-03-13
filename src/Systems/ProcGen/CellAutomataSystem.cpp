
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys::ProcGen
{

void CellAutomataSystem::iterate( unsigned int iterations, RandomLevelGenerator::SceneType scene_type,
                                  PathFinding::SpatialHashGrid &levelgen_spatialgrid )
{
  sf::Clock iteration_timer;
  for ( unsigned int i = 0; i < iterations; i++ )
  {

    SPDLOG_DEBUG( "NAVMESH: {}", levelgen_spatialgrid.size() );
    for ( auto [pos_entt, pos_cmp] : getReg().view<Cmp::Position>().each() )
    {
      if ( getReg().any_of<Cmp::ReservedPosition>( pos_entt ) ) continue;
      std::vector<entt::entity> neighbour_list = levelgen_spatialgrid.neighbours( pos_cmp );
      SPDLOG_DEBUG( "#{} at {},{} has {} nieghbours", static_cast<uint32_t>( pos_entt ), pos_cmp.x(), pos_cmp.y(), neighbour_list.size() );

      if ( neighbour_list.size() <= 2 )
      {
        if ( scene_type == RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR )
        {
          Factory::create_obstacle( getReg(), pos_entt, pos_cmp, "ROCK", 0, pos_cmp.position.y );
        }
        else if ( scene_type == RandomLevelGenerator::SceneType::CRYPT_INTERIOR )
        {
          auto [type, idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
          Factory::create_obstacle( getReg(), pos_entt, pos_cmp, type, idx, pos_cmp.position.y );
        }
      }
      else if ( neighbour_list.size() > 2 and neighbour_list.size() < 5 )
      {
        //
        Factory::remove_obstacle( getReg(), pos_entt );
      }
      else
      {
        if ( scene_type == RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR )
        {
          Factory::create_obstacle( getReg(), pos_entt, pos_cmp, "ROCK", 0, pos_cmp.position.y );
        }
        else if ( scene_type == RandomLevelGenerator::SceneType::CRYPT_INTERIOR )
        {
          auto [type, idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
          Factory::create_obstacle( getReg(), pos_entt, pos_cmp, type, idx, pos_cmp.position.y );
        }
      }
    }
    SPDLOG_INFO( "Iteration #{} took {}ms", i, iteration_timer.restart().asMilliseconds() );
  }

  SPDLOG_DEBUG( "Total Iterations: {}", iterations );
}

} // namespace ProceduralMaze::Sys::ProcGen