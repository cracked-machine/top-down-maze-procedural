#include <Crypt/CryptPassageDoor.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Events/PassageEvent.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/ProcGen/PassageSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace ProceduralMaze::Sys
{

void PassageSystem::update( [[maybe_unused]] sf::Time dt )
{
  if ( m_connect_all_rooms ) { create_cached_passages(); }
}

void PassageSystem::on_passage_event( Events::PassageEvent &event )
{
  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data ) std::runtime_error( "Unable to lock Scene::SceneConfigSharedPtr" );
  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();

  switch ( event.type )
  {
    case Events::PassageEvent::Type::REMOVE_PASSAGES:
      m_passage_algos.reset();
      remove_passages();
      break;
    case Events::PassageEvent::Type::OPEN_PASSAGES:
      open_passages();
      break;
    case Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS:
      connect_start_and_open_rooms_passages( event.entity );
      break;
    case Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_OPENROOMS:
      connect_occupied_and_open_room_passages();
      break;
    case Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_ENDROOM:
      connect_occupied_and_end_room_passages( getReg(), event.entity, map_size_pixel );
      tidy_passage_blocks();
      create_uncached_passages();
      break;
    case Events::PassageEvent::Type::CACHE_ALL_ROOM_CONNECTIONS:
      m_region_idx = 0;
      cache_all_room_connections();
      break;
    case Events::PassageEvent::Type::CONNECT_ALL_ROOMS:
      m_connect_all_rooms = true;
      break;
    case Events::PassageEvent::Type::ADD_SPIKE_TRAPS:
      add_spike_traps();
      break;
  }
}

void PassageSystem::remove_passages()
{
  fill_all_passages();
  remove_all_passage_blocks();
  m_uncached_passage_list.clear();
}

void PassageSystem::open_passages()
{
  //
  empty_open_passages();
}

void PassageSystem::add_spike_traps()
{
  auto passage_picker = Cmp::RandomInt( 0, m_passage_algos.get_current_passage_id() );
  // static int max_num_spike_traps = 3;
  std::set<int> passage_ids_used;

  auto pblock_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [pblock_entt, pblock_cmp] : pblock_view.each() )
  {
    if ( passage_ids_used.contains( pblock_cmp.m_passage_id ) ) continue;
    passage_ids_used.insert( pblock_cmp.m_passage_id );
    Factory::add_spike_trap( getReg(), pblock_entt, pblock_cmp.m_passage_id );
  }
}

void PassageSystem::connect_start_and_open_rooms_passages( entt::entity start_room_entt )
{
  if ( start_room_entt == entt::null )
  {
    SPDLOG_WARN( "start_room_entt is null" );
    return;
  }
  auto start_room_cmp = getReg().try_get<Cmp::CryptRoomStart>( start_room_entt );
  if ( not start_room_cmp )
  {
    SPDLOG_WARN( "start_room_cmp is null" );
    return;
  }

  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data ) std::runtime_error( "Unable to lock Scene::SceneConfigSharedPtr" );
  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();

  const auto start_room_right_pos_x = start_room_cmp->position.x + start_room_cmp->size.x;

  // divide the gamrarea into 3 quadrants - again there are only three because startroom is southern most position in the game area
  auto west_quad = sf::FloatRect( { 0.f, 0.f }, { start_room_cmp->position.x, map_size_pixel.y } );
  auto east_quad = sf::FloatRect( { start_room_right_pos_x, 0.f }, { map_size_pixel.x - ( start_room_right_pos_x ), map_size_pixel.y } );
  auto north_quad = sf::FloatRect( { 0.f, 0.f }, { map_size_pixel.x, start_room_cmp->position.y } );

  std::vector<std::pair<Cmp::CryptPassageDirection, sf::FloatRect>> quadrants = { { Cmp::CryptPassageDirection::WEST, west_quad },
                                                                                  { Cmp::CryptPassageDirection::EAST, east_quad },
                                                                                  { Cmp::CryptPassageDirection::NORTH, north_quad } };

  for ( auto &[direction, qaudrant] : quadrants )
  {
    auto distances = find_room_distances<Cmp::CryptRoomOpen>( start_room_cmp->m_connectors[direction], qaudrant, { start_room_entt } );
    auto passage_blocks = find_passages<Cmp::CryptRoomOpen>( start_room_cmp->m_connectors[direction], distances, ProcGen::WalkingType::DRUNK,
                                                             map_size_pixel, ProcGen::OnePassagePerTargetRoom::YES,
                                                             ProcGen::AllowDuplicatePassages::NO );
    m_uncached_passage_list.insert( m_uncached_passage_list.begin(), passage_blocks.begin(), passage_blocks.end() );
  }

  create_uncached_passages();
  tidy_passage_blocks();
}

void PassageSystem::connect_occupied_and_open_room_passages()
{
  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data ) std::runtime_error( "Unable to lock Scene::SceneConfigSharedPtr" );

  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();

  // find the open room that the player is in (if any)
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::Player::get_position( getReg() ).findIntersection( open_room_cmp ) ) continue;

    auto &occupied_room_cmp = open_room_cmp;

    const auto current_room_right_pos_x = occupied_room_cmp.position.x + occupied_room_cmp.size.x;
    const auto current_room_bottom_pos_y = occupied_room_cmp.position.y + occupied_room_cmp.size.y;

    // divide the gamrarea into 4 quadrants
    auto west_quad = sf::FloatRect( { 0.f, 0.f }, { occupied_room_cmp.position.x, map_size_pixel.y } );
    auto east_quad = sf::FloatRect( { current_room_right_pos_x, 0.f }, { map_size_pixel.x - ( current_room_right_pos_x ), map_size_pixel.y } );
    auto north_quad = sf::FloatRect( { 0.f, 0.f }, { map_size_pixel.x, occupied_room_cmp.position.y } );
    auto south_quad = sf::FloatRect( { occupied_room_cmp.position.x, current_room_bottom_pos_y },
                                     { current_room_right_pos_x, map_size_pixel.y - ( current_room_bottom_pos_y ) } );

    std::vector<std::pair<Cmp::CryptPassageDirection, sf::FloatRect>> quadrants = { { Cmp::CryptPassageDirection::WEST, west_quad },
                                                                                    { Cmp::CryptPassageDirection::EAST, east_quad },
                                                                                    { Cmp::CryptPassageDirection::NORTH, north_quad },
                                                                                    { Cmp::CryptPassageDirection::SOUTH, south_quad } };

    for ( auto &[direction, qaudrant] : quadrants )
    {
      auto distances = find_room_distances<Cmp::CryptRoomOpen>( occupied_room_cmp.m_connectors[direction], qaudrant, { open_room_entt } );
      auto passage_blocks = find_passages<Cmp::CryptRoomOpen>( occupied_room_cmp.m_connectors[direction], distances, ProcGen::WalkingType::DRUNK,
                                                               map_size_pixel, ProcGen::OnePassagePerTargetRoom::YES,
                                                               ProcGen::AllowDuplicatePassages::NO );
      m_uncached_passage_list.insert( m_uncached_passage_list.begin(), passage_blocks.begin(), passage_blocks.end() );
    }
    create_uncached_passages();
    tidy_passage_blocks();
  }
}

void PassageSystem::connect_occupied_and_end_room_passages( entt::registry &reg, entt::entity end_room_entt, sf::Vector2f map_size_pixel )
{
  std::vector<Cmp::CryptPassageBlock> passage_block_list;
  if ( end_room_entt == entt::null )
  {
    SPDLOG_WARN( "End room entt is null" );
    return;
  }
  auto crypt_end_room_cmp = reg.try_get<Cmp::CryptRoomEnd>( end_room_entt );
  if ( not crypt_end_room_cmp )
  {
    SPDLOG_WARN( "end room cmp is null" );
    return;
  }

  auto open_room_view = reg.view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::Player::get_position( reg ).findIntersection( open_room_cmp ) ) continue;
    auto &occupied_room_cmp = open_room_cmp;

    // no need to search for suitable target, we already have it
    auto current_passage_door = occupied_room_cmp.m_connectors[Cmp::CryptPassageDirection::NORTH];
    m_passage_algos.increment_passage_id();

    std::vector<Cmp::CryptPassageBlock> passage_blocks = m_passage_algos.create_drunken_walk( reg, current_passage_door, *crypt_end_room_cmp,
                                                                                              map_size_pixel, { open_room_entt, end_room_entt } );

    m_uncached_passage_list.insert( m_uncached_passage_list.begin(), passage_blocks.begin(), passage_blocks.end() );
  }
}

void PassageSystem::cache_all_room_connections()
{

  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data ) std::runtime_error( "Unable to lock Scene::SceneConfigSharedPtr" );

  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();
  const auto world_area = sf::FloatRect( { 0, 0 }, map_size_pixel );

  auto closed_room_view = getReg().view<Cmp::CryptRoomClosed>();
  SPDLOG_INFO( "CryptRoomClosed count {}", closed_room_view.size() );

  std::vector<Cmp::CryptPassageDirection> directions = { Cmp::CryptPassageDirection::WEST, Cmp::CryptPassageDirection::EAST,
                                                         Cmp::CryptPassageDirection::NORTH, Cmp::CryptPassageDirection::SOUTH };

  float region_height = world_area.size.y / m_cached_passage_list.size();
  float region_width = world_area.size.x;
  SPDLOG_INFO( "World height: {}, Region Height: {}", world_area.size.y, region_height );
  for ( auto [idx, it] : std::views::enumerate( m_cached_passage_list ) )
  {
    auto new_region = sf::FloatRect( { 0.f, idx * region_height }, { region_width, region_height } );
    it = ProcGen::PassageCachedRegions<40>::BlockRegion{ new_region, {} };
    SPDLOG_INFO( "Created cached region {},{} {},{}", new_region.position.x, new_region.position.y, new_region.size.x, new_region.size.y );
  }

  // Do this first to guarantee success otherwise player cannot leave
  connect_end_room_to_nearest_closed_room();

  for ( auto [closed_room_entt, closed_room_cmp] : closed_room_view.each() )
  {
    auto &current_room_cmp = closed_room_cmp;
    for ( auto &direction : directions )
    {
      auto distances = find_room_distances<Cmp::CryptRoomClosed>( current_room_cmp.m_connectors[direction], world_area, { closed_room_entt } );
      auto passage_blocks = find_passages<Cmp::CryptRoomClosed>( current_room_cmp.m_connectors[direction], distances, ProcGen::WalkingType::DRUNK,
                                                                 map_size_pixel, ProcGen::OnePassagePerTargetRoom::YES,
                                                                 ProcGen::AllowDuplicatePassages::NO );
      if ( not passage_blocks.empty() ) m_passage_algos.increment_passage_id();

      for ( auto &passage_block_cmp : passage_blocks )
      {
        for ( auto &[region, blocklist] : m_cached_passage_list )
        {
          if ( not sf::FloatRect( passage_block_cmp, Constants::kGridSizePxF ).findIntersection( region ) ) continue;
          blocklist.push_back( passage_block_cmp );
        }
      }
    }
  }

  SPDLOG_INFO( "BlockRegion count {}", m_cached_passage_list.size() );

  // tidyPassageBlocks();
}

void PassageSystem::connect_end_room_to_nearest_closed_room()
{
  auto end_room_view = getReg().view<Cmp::CryptRoomEnd>();
  if ( end_room_view->empty() )
  {
    SPDLOG_WARN( "no end room found" );
    return;
  }

  auto end_room_entt = *end_room_view.begin();
  auto &end_room_cmp = end_room_view.get<Cmp::CryptRoomEnd>( end_room_entt );

  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data )
  {
    SPDLOG_ERROR( "crypt_scene_data is null" );
    return;
  }
  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();
  const auto world_area = sf::FloatRect( { 0.f, 0.f }, map_size_pixel );

  SPDLOG_INFO( "connecting end room {} to nearest closed room", static_cast<uint32_t>( end_room_entt ) );

  // Try only south door as CryptRoomEnd only appears at the top of the game area, stop as soon as one walk succeeds
  const std::array directions = { Cmp::CryptPassageDirection::SOUTH };
  for ( auto direction : directions )
  {
    auto distances = find_room_distances<Cmp::CryptRoomClosed>( end_room_cmp.m_connectors[direction], world_area, { end_room_entt } );
    auto passage_blocks = find_passages<Cmp::CryptRoomClosed>( end_room_cmp.m_connectors[direction], distances, ProcGen::WalkingType::DRUNK,
                                                               map_size_pixel, ProcGen::OnePassagePerTargetRoom::YES,
                                                               ProcGen::AllowDuplicatePassages::NO );
    if ( not passage_blocks.empty() ) m_passage_algos.increment_passage_id();

    for ( auto &passage_block_cmp : passage_blocks )
    {
      for ( auto &[region, blocklist] : m_cached_passage_list )
      {
        if ( not sf::FloatRect( passage_block_cmp, Constants::kGridSizePxF ).findIntersection( region ) ) continue;
        blocklist.push_back( passage_block_cmp );
      }
    }
  }
}

void PassageSystem::create_uncached_passages()
{
  for ( auto &passage_block_cmp : m_uncached_passage_list )
  {
    auto entt = getReg().create();
    getReg().emplace<Cmp::CryptPassageBlock>( entt, passage_block_cmp );
  }
}

void PassageSystem::create_cached_passages()
{

  if ( m_region_idx >= m_cached_passage_list.size() )
  {
    m_connect_all_rooms = false;
    open_passages();
    add_spike_traps();
    return;
  }

  auto [region, blocklist] = m_cached_passage_list.get( m_region_idx );
  SPDLOG_INFO( "Spawning region {} with {} blocks", m_region_idx, blocklist.size() );
  for ( auto &block : blocklist )
  {
    auto entt = getReg().create();
    getReg().emplace<Cmp::CryptPassageBlock>( entt, block );
  }
  m_region_idx++;
}

template <typename ROOMTYPE>
ProcGen::MidPointDistanceQueue PassageSystem::find_room_distances( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant,
                                                                   std::set<entt::entity> exclude_entts )
{

  ProcGen::MidPointDistanceQueue pqueue;
  for ( auto [other_room_entt, other_room_cmp] : getReg().view<ROOMTYPE>().each() )
  {
    if ( exclude_entts.contains( other_room_entt ) ) continue;
    if ( not other_room_cmp.findIntersection( search_quadrant ) ) continue;
    if ( other_room_cmp.are_all_doors_used() ) continue;
    auto distance = Utils::Maths::getEuclideanDistance( start_passage_door, other_room_cmp.getCenter() );
    SPDLOG_DEBUG( "Found room {} at distance {}", static_cast<uint32_t>( other_room_entt ), distance );
    pqueue.push( { distance, other_room_entt } );
  }

  return pqueue;
}
template ProcGen::MidPointDistanceQueue PassageSystem::find_room_distances<Cmp::CryptRoomOpen>( Cmp::CryptPassageDoor &, const sf::FloatRect,
                                                                                                std::set<entt::entity> );
template ProcGen::MidPointDistanceQueue PassageSystem::find_room_distances<Cmp::CryptRoomClosed>( Cmp::CryptPassageDoor &, const sf::FloatRect,
                                                                                                  std::set<entt::entity> );

template <typename ROOMTYPE>
std::vector<Cmp::CryptPassageBlock> PassageSystem::find_passages( Cmp::CryptPassageDoor &start_passage_door,
                                                                  ProcGen::MidPointDistanceQueue &dist_pqueue, ProcGen::WalkingType walktype,
                                                                  sf::Vector2f map_size_pixel, ProcGen::OnePassagePerTargetRoom passage_limit,
                                                                  ProcGen::AllowDuplicatePassages duplicates_policy )
{
  std::vector<Cmp::CryptPassageBlock> final_passage_list;

  // process the distance list, one room at a time
  while ( not dist_pqueue.empty() )
  {
    auto nearest_other_room_entt = dist_pqueue.top().second;
    dist_pqueue.pop();
    if ( not getReg().valid( nearest_other_room_entt ) ) continue;

    auto *other_room_bounds = getReg().try_get<ROOMTYPE>( nearest_other_room_entt );
    if ( not other_room_bounds ) continue;

    std::vector<Cmp::CryptPassageBlock> passage_list;

    // try to create a room-to-room pathway
    if ( walktype == ProcGen::WalkingType::DRUNK )
    {
      passage_list = m_passage_algos.create_drunken_walk( getReg(), start_passage_door, *other_room_bounds, map_size_pixel,
                                                          { nearest_other_room_entt }, duplicates_policy );
    }
    else { passage_list = m_passage_algos.create_dog_leg( getReg(), start_passage_door, *other_room_bounds, duplicates_policy ); }

    // if the pathway was successfull in reaching goal
    if ( not passage_list.empty() )
    {
      auto *room_cmp = getReg().try_get<ROOMTYPE>( nearest_other_room_entt );
      if ( passage_limit == ProcGen::OnePassagePerTargetRoom::YES && room_cmp ) { room_cmp->set_all_doors_used( true ); }

      final_passage_list.insert( final_passage_list.end(), passage_list.begin(), passage_list.end() );

      break;
    }
  }
  return final_passage_list;
}

template std::vector<Cmp::CryptPassageBlock> PassageSystem::find_passages<Cmp::CryptRoomOpen>( Cmp::CryptPassageDoor &,
                                                                                               ProcGen::MidPointDistanceQueue &, ProcGen::WalkingType,
                                                                                               sf::Vector2f, ProcGen::OnePassagePerTargetRoom,
                                                                                               ProcGen::AllowDuplicatePassages );

template std::vector<Cmp::CryptPassageBlock>
PassageSystem::find_passages<Cmp::CryptRoomClosed>( Cmp::CryptPassageDoor &, ProcGen::MidPointDistanceQueue &, ProcGen::WalkingType, sf::Vector2f,
                                                    ProcGen::OnePassagePerTargetRoom, ProcGen::AllowDuplicatePassages );

/// PRIVATE FUNCTIONS

void PassageSystem::remove_all_passage_blocks()
{
  std::vector<entt::entity> passage_block_remove_list;

  // find all Cmp::CryptPassageBlocks
  auto crypt_passage_block_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [entt, block_cmp] : crypt_passage_block_view.each() )
  {
    passage_block_remove_list.push_back( entt );
  }

  // Remove Cmp::CryptPassageBlocks safely
  for ( auto entt : passage_block_remove_list )
  {
    getReg().remove<Cmp::CryptPassageBlock>( entt );
    getReg().destroy( entt );
  }
}

void PassageSystem::empty_open_passages()
{
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    auto pblock_view = getReg().view<Cmp::CryptPassageBlock>();
    for ( auto [pblock_entt, pblock_cmp] : pblock_view.each() )
    {
      auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSizePxF );
      // skip any positions that are not pblocks or do not have obstacles
      if ( not pblock_cmp_rect.findIntersection( pos_cmp ) ) continue;
      if ( not getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      Factory::remove_obstacle( getReg(), pos_entt );
      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
      {
        pathfinding_navmesh->insert( pos_entt, pos_cmp );
      }
    }
  }
}

void PassageSystem::fill_all_passages()
{

  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    // don't add obstacles to footstep entities
    if ( getReg().any_of<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Direction>( pos_entt ) ) continue;

    auto pblock_view = getReg().view<Cmp::CryptPassageBlock>();
    for ( auto [pblock_entt, pblock_cmp] : pblock_view.each() )
    {
      auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSizePxF );
      // skip any positions that are not pblocks or already have obstacles
      if ( not pblock_cmp_rect.findIntersection( pos_cmp ) ) continue;
      if ( getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_sb" );
      Factory::create_obstacle( getReg(), pos_entt, pos_cmp, ms, 2 );

      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
      {
        pathfinding_navmesh->remove( pos_entt, pos_cmp );
      }

      if ( not Utils::getSystemCmp( getReg() ).collisions_disabled )
      {
        if ( Utils::Player::get_position( getReg() ).findIntersection( pblock_cmp_rect ) )
        {
          // player got squished
          get_systems_event_queue().enqueue(
              Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SQUISHED, Utils::Player::get_position( getReg() ) ) );
        }
      }
    }
  }
}

void PassageSystem::tidy_passage_blocks( bool include_closed_rooms )
{
  for ( auto [pblock_entt, pblock_cmp] : getReg().view<Cmp::CryptPassageBlock>().each() )
  {
    auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSizePxF );

    // open rooms
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( pblock_cmp_rect.findIntersection( open_room_cmp ) ) getReg().remove<Cmp::CryptPassageBlock>( pblock_entt );
    }

    // closed rooms - this can interfere with passage creation so normal usescases don't need it
    if ( include_closed_rooms )
    {
      for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
      {
        if ( pblock_cmp_rect.findIntersection( closed_room_cmp ) ) getReg().remove<Cmp::CryptPassageBlock>( pblock_entt );
      }
    }

    // start rooms
    for ( auto [start_room_entt, start_room_cmp] : getReg().view<Cmp::CryptRoomStart>().each() )
    {
      if ( pblock_cmp_rect.findIntersection( start_room_cmp ) ) getReg().remove<Cmp::CryptPassageBlock>( pblock_entt );
    }

    // end rooms
    for ( auto [end_room_entt, end_room_cmp] : getReg().view<Cmp::CryptRoomEnd>().each() )
    {
      if ( pblock_cmp_rect.findIntersection( end_room_cmp ) ) getReg().remove<Cmp::CryptPassageBlock>( pblock_entt );
    }
  }
}

} // namespace ProceduralMaze::Sys