
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
  const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "ROCK" );

  sf::Clock iteration_timer;
  for ( unsigned int i = 0; i < iterations; i++ )
  {

    SPDLOG_DEBUG( "NAVMESH: {}", levelgen_spatialgrid.size() );
    for ( auto [pos_entt, pos_cmp] : reg().view<Cmp::Position>().each() )
    {
      if ( reg().any_of<Cmp::ReservedPosition>( pos_entt ) ) continue;
      std::vector<entt::entity> neighbour_list = levelgen_spatialgrid.neighbours( pos_cmp );
      SPDLOG_DEBUG( "#{} at {},{} has {} nieghbours", static_cast<uint32_t>( pos_entt ), pos_cmp.x(), pos_cmp.y(), neighbour_list.size() );

      if ( neighbour_list.size() <= 2 )
      {
        if ( scene_type == RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR )
        {
          auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "ROCK" } );
          Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, idx );
        }
        else if ( scene_type == RandomLevelGenerator::SceneType::CRYPT_INTERIOR )
        {
          auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
          Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, idx );
        }
      }
      else if ( neighbour_list.size() > 2 and neighbour_list.size() < 5 )
      {
        //
        Factory::remove_obstacle( reg(), pos_entt );
      }
      else
      {

        if ( scene_type == RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR )
        {
          auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "ROCK" } );
          Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, idx );
        }
        else if ( scene_type == RandomLevelGenerator::SceneType::CRYPT_INTERIOR )
        {
          auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
          Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, idx );
        }
      }
    }
    SPDLOG_INFO( "Iteration #{} took {}ms", i, iteration_timer.restart().asMilliseconds() );
  }

  SPDLOG_DEBUG( "Total Iterations: {}", iterations );
}

} // namespace ProceduralMaze::Sys::ProcGen