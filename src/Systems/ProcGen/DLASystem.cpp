#include <Utils/Constants.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Components/Random.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Systems/ProcGen/DLASystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Utils.hpp>

namespace Game::Sys::ProcGen
{

void DLASystem::iterate( sf::FloatRect scene_size, const sf::Vector2f seed_pos, uint16_t particle_limit,
                         PathFinding::SpatialHashGrid &levelgen_spatialgrid )
{
  // clang-format off
  static const std::array<sf::Vector2f, 4> cardinal_offsets = {
      sf::Vector2f{ Constants::kGridSizePxF.x, 0.f }, 
      sf::Vector2f{ -Constants::kGridSizePxF.x, 0.f }, 
      sf::Vector2f{ 0.f, Constants::kGridSizePxF.y },
      sf::Vector2f{ 0.f, -Constants::kGridSizePxF.y } };
  // clang-format on

  // Carve the seed at center
  auto center_entities = levelgen_spatialgrid.at( Cmp::Position( seed_pos, Constants::kGridSizePxF ) );
  for ( auto &entt : center_entities )
  {
    Factory::remove_obstacle( reg(), entt );
    levelgen_spatialgrid.remove( entt, Cmp::Position( seed_pos, Constants::kGridSizePxF ) );
  }
  SPDLOG_INFO( "Seed particle position: {},{}", seed_pos.x, seed_pos.y );

  Cmp::RandomInt angle_picker( 0, 359 );
  Cmp::RandomInt dir_picker( 0, 3 );

  // Random position pickers across the full scene interior (inset 1 cell)
  Cmp::RandomInt rand_col( 1, static_cast<int>( ( scene_size.size.x / Constants::kGridSizePxF.x ) ) - 2 );
  Cmp::RandomInt rand_row( 1, static_cast<int>( ( scene_size.size.y / Constants::kGridSizePxF.y ) ) - 2 );

  int particle_count = 0;
  while ( particle_count < particle_limit )
  {
    // Spawn at a uniformly random grid cell inside the scene
    sf::Vector2f spawn_pos = scene_size.position +
                             sf::Vector2f( rand_col.gen() * Constants::kGridSizePxF.x, rand_row.gen() * Constants::kGridSizePxF.y );

    Cmp::Position particle( Utils::snap_to_grid( spawn_pos ), Constants::kGridSizePxF );

    bool stuck = false;
    int steps = 0;
    const int max_steps = 10000; // prevent truly runaway walkers

    while ( not stuck and steps < max_steps )
    {
      ++steps;

      if ( not scene_size.contains( particle.position ) ) break;

      // Check if adjacent to a carved (empty) cell
      bool adjacent_to_aggregate = false;
      for ( const auto &offset : cardinal_offsets )
      {
        sf::Vector2f neighbour_world = Utils::snap_to_grid( particle.position + offset );
        if ( not scene_size.contains( neighbour_world ) ) continue;
        Cmp::Position neighbour( neighbour_world, Constants::kGridSizePxF );
        if ( levelgen_spatialgrid.at( neighbour ).empty() )
        {
          adjacent_to_aggregate = true;
          break;
        }
      }

      if ( adjacent_to_aggregate )
      {
        auto entities_here = levelgen_spatialgrid.at( particle );
        for ( auto &entt : entities_here )
        {
          Factory::remove_obstacle( reg(), entt );
          levelgen_spatialgrid.remove( entt, particle );
        }
        stuck = true;
        ++particle_count;
        SPDLOG_INFO( "Carved {},{} particle {}/{}", particle.x(), particle.y(), particle_count, particle_limit );
      }
      else
      {
        sf::Vector2f new_pos = particle.position + cardinal_offsets[dir_picker.gen()];
        particle = Cmp::Position( Utils::snap_to_grid( new_pos ), Constants::kGridSizePxF );
      }
    }
  }
}

} // namespace Game::Sys::ProcGen