#include <Components/NoPathFinding.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ZOrderValue.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys::ProcGen
{

void CellAutomataSystem::iterate( unsigned int iterations )
{
  sf::Clock iteration_timer;
  for ( unsigned int i = 0; i < iterations; i++ )
  {
    find_neighbours();
    apply_rules();
    SPDLOG_INFO( "#{} took {}ms", i, iteration_timer.restart().asMilliseconds() );
  }

  // run one last time to get the latest neighbour data
  find_neighbours();
  SPDLOG_DEBUG( "Total Iterations: {}", iterations );
}

void CellAutomataSystem::find_neighbours()
{
  // 1. find neighbours
  for ( auto it = m_random_level->begin(); it != m_random_level->end(); it++ )
  {
    auto current_entity = entt::entity( *it );

    // Skip invalid entities completely
    if ( !getReg().valid( current_entity ) )
    {
      SPDLOG_WARN( "Skipping invalid entity {}", entt::to_integral( *it ) );
      continue;
    }

    // Clear previous neighbours
    if ( not getReg().all_of<Cmp::Neighbours>( current_entity ) )
    {
      // SPDLOG_WARN( "Entity {} does not have Neighbours component, skipping", entt::to_integral( *it ) );
      continue;
    }
    getReg().patch<Cmp::Neighbours>( current_entity, []( auto &_nb_update ) { _nb_update.clear(); } );

    // calculate game area boundary edges within the `m_random_level` linear vector
    const int idx = std::distance( m_random_level->begin(), it );
    bool has_left_map_edge = not( ( idx ) % kMapGridSize.y );
    bool has_right_map_edge = not( ( idx + 1 ) % kMapGridSize.y );

    // ---------------------------------------
    //   Columns
    // ---------------------------------------
    // |  N - (y-1) |   N - 1   |            |  Rows
    // ---------------------------------------
    // |   N - y    |     N     |            |
    // ---------------------------------------
    // | N - (y+1)  |           |            |
    // ---------------------------------------
    // where N is iterator, y is column length

    // N - 1
    if ( std::prev( it ) >= m_random_level->begin() )
    {
      auto left_entt = entt::entity( *std::prev( it ) );
      Cmp::Obstacle *left_entt_ob = getReg().try_get<Cmp::Obstacle>( left_entt );
      if ( left_entt_ob && not has_left_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity, [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::LEFT, left_entt ); } );
      }
    }
    // N - (y - 1)
    if ( std::prev( it, ( kMapGridSize.y + 1 ) ) >= m_random_level->begin() )
    {
      auto down_left_entt = entt::entity( *std::prev( it, kMapGridSize.y + 1 ) );
      Cmp::Obstacle *down_left_entt_ob = getReg().try_get<Cmp::Obstacle>( down_left_entt );
      if ( down_left_entt_ob && not has_left_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity,
                                         [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::DOWN_LEFT, down_left_entt ); } );
      }
    }
    // N - y
    if ( std::prev( it, kMapGridSize.y ) >= m_random_level->begin() )
    {
      auto down_entt = entt::entity( *std::prev( it, kMapGridSize.y ) );
      Cmp::Obstacle *down_entt_ob = getReg().try_get<Cmp::Obstacle>( down_entt );
      if ( down_entt_ob )
      {
        getReg().patch<Cmp::Neighbours>( current_entity, [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::DOWN, down_entt ); } );
      }
    }
    // N - (y + 1)
    if ( ( std::prev( it, ( kMapGridSize.y - 1 ) ) ) >= m_random_level->begin() )
    {
      auto down_right_entt = entt::entity( *std::prev( it, kMapGridSize.y - 1 ) );
      Cmp::Obstacle *down_right_entt_ob = getReg().try_get<Cmp::Obstacle>( down_right_entt );
      if ( down_right_entt_ob && not has_right_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity,
                                         [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::DOWN_RIGHT, down_right_entt ); } );
      }
    }

    //   Columns
    // ---------------------------------------
    // |            |           |  N + (y-1) |  Rows
    // ---------------------------------------
    // |            |     N     |    N + y   |
    // ---------------------------------------
    // |            |   N + 1   | N + (y+1)  |
    // ---------------------------------------

    // where N is iterator, y is column length

    // N + (y - 1)
    if ( std::next( it, ( kMapGridSize.y - 1 ) ) < m_random_level->end() )
    {
      auto top_left_entt = entt::entity( *std::next( it, kMapGridSize.y - 1 ) );
      Cmp::Obstacle *top_left_entt_ob = getReg().try_get<Cmp::Obstacle>( top_left_entt );
      if ( top_left_entt_ob && not has_left_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity,
                                         [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::UP_LEFT, top_left_entt ); } );
      }
    }
    // N + y
    if ( kMapGridSize.y < m_random_level->size() && std::next( it, kMapGridSize.y ) < m_random_level->end() )
    {
      auto top_entt = entt::entity( *std::next( it, kMapGridSize.y ) );
      Cmp::Obstacle *top_entt_ob = getReg().try_get<Cmp::Obstacle>( top_entt );
      if ( top_entt_ob )
      {
        getReg().patch<Cmp::Neighbours>( current_entity, [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::UP, top_entt ); } );
      }
    }
    // N + (y + 1)
    if ( ( kMapGridSize.y + 1 ) < m_random_level->size() && std::next( it, ( kMapGridSize.y + 1 ) ) < m_random_level->end() )
    {
      auto top_right_entt = entt::entity( *std::next( it, ( kMapGridSize.y + 1 ) ) );
      Cmp::Obstacle *top_right_entt_ob = getReg().try_get<Cmp::Obstacle>( top_right_entt );
      if ( top_right_entt_ob && not has_right_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity,
                                         [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::UP_RIGHT, top_right_entt ); } );
      }
    }
    // N + 1
    if ( std::next( it ) < m_random_level->end() )
    {
      auto right_entt = entt::entity( *std::next( it ) );
      Cmp::Obstacle *right_entt_ob = getReg().try_get<Cmp::Obstacle>( right_entt );
      if ( right_entt_ob && not has_right_map_edge )
      {
        getReg().patch<Cmp::Neighbours>( current_entity, [&]( auto &_nb_update ) { _nb_update.set( Cmp::Neighbours::Dir::RIGHT, right_entt ); } );
      }
    }
  }
  SPDLOG_DEBUG( "Processed neighbours for {} entities.", m_random_level->size() );

#ifdef NDEBUG

  auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>();
  for ( auto [entity, obstacle_cmp, pos_cmp, neighbour_cmp] : obstacle_view.each() )
  {
    // SPDLOG_INFO("Entity {} has {} neighbours", entt::to_integral(_entt),
    // _nb.count());
    std::string msg = std::to_string( entt::to_integral( entity ) ) + "(" + std::to_string( neighbour_cmp.count() ) + ") = ";

    for ( auto [_dir, _nb_entt] : neighbour_cmp )
    {
      msg += "[" + neighbour_cmp.to_string( _dir ) + ":" + std::to_string( entt::to_integral( _nb_entt ) ) + "] ";
    }
    SPDLOG_TRACE( msg );
  }
#endif
}

void CellAutomataSystem::apply_rules()
{
  // 2. apply rules
  auto non_reserved_view = getReg().view<Cmp::Position, Cmp::Neighbours>( entt::exclude<Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp, neighbour_cmp] : non_reserved_view.each() )
  {
    if ( neighbour_cmp.count() <= 2 )
    {
      getReg().emplace_or_replace<Cmp::Obstacle>( entity );
      getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y );
      getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
    }
    else if ( neighbour_cmp.count() > 2 and neighbour_cmp.count() < 5 )
    {
      if ( getReg().all_of<Cmp::Obstacle>( entity ) ) { getReg().remove<Cmp::Obstacle>( entity ); }
      if ( getReg().all_of<Cmp::NoPathFinding>( entity ) ) { getReg().remove<Cmp::NoPathFinding>( entity ); }
      if ( getReg().all_of<Cmp::ZOrderValue>( entity ) ) { getReg().remove<Cmp::ZOrderValue>( entity ); }
    }
    else
    {
      getReg().emplace_or_replace<Cmp::Obstacle>( entity );
      getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y );
      getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
    }
  }
  SPDLOG_DEBUG( "Finished applying Cellular Automata rules!" );
}

} // namespace ProceduralMaze::Sys::ProcGen