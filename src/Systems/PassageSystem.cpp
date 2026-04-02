#include <Constants.hpp>
#include <Systems/BaseSystem.hpp>
#include <optional>
#include <stdexcept>
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
#include <Systems/Events/PassageEvent.hpp>
#include <Systems/PassageSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <queue>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

void PassageSystem::update( [[maybe_unused]] sf::Time dt )
{
  if ( m_connect_all_rooms ) { create_cached_passages(); }
}

void PassageSystem::on_passage_event( Events::PassageEvent &event )
{
  switch ( event.type )
  {
    case Events::PassageEvent::Type::REMOVE_PASSAGES:
      m_current_passage_id = 0;
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
      connect_occupied_and_end_room_passages( event.entity );
      break;
    case Events::PassageEvent::Type::CACHE_ALL_ROOM_CONNECTIONS:
      m_region_idx = 0;
      cache_all_room_connections();
      break;
    case Events::PassageEvent::Type::CONNECT_ALL_ROOMS:
      // create_cached_passages();
      m_connect_all_rooms = true;
      break;
    case Events::PassageEvent::Type::ADD_SPIKE_TRAPS:
      add_spike_traps();
      break;
  }
}

void PassageSystem::remove_passages()
{
  fillAllPassages();
  removeAllPassageBlocks();
  m_uncached_passage_list.clear();
}

void PassageSystem::open_passages()
{
  //
  emptyOpenPassages();
}

void PassageSystem::add_spike_traps()
{
  auto passage_picker = Cmp::RandomInt( 0, m_current_passage_id );
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

  find_passage_target( start_room_cmp->m_passageconnections[Cmp::CryptPassageDirection::WEST], west_quad, { start_room_entt } );
  find_passage_target( start_room_cmp->m_passageconnections[Cmp::CryptPassageDirection::EAST], east_quad, { start_room_entt } );
  find_passage_target( start_room_cmp->m_passageconnections[Cmp::CryptPassageDirection::NORTH], north_quad, { start_room_entt } );
  create_uncached_passages();
  tidyPassageBlocks();
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

    find_passage_target( occupied_room_cmp.m_passageconnections[Cmp::CryptPassageDirection::WEST], west_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_passageconnections[Cmp::CryptPassageDirection::EAST], east_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_passageconnections[Cmp::CryptPassageDirection::NORTH], north_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_passageconnections[Cmp::CryptPassageDirection::SOUTH], south_quad, { open_room_entt } );
    create_uncached_passages();
    tidyPassageBlocks();
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

  float region_height = world_area.size.y / kMaxRegions;
  float region_width = world_area.size.x;
  SPDLOG_INFO( "World height: {}, Region Height: {}", world_area.size.y, region_height );
  for ( auto r : std::views::iota( 0, kMaxRegions ) )
  {
    auto new_region = sf::FloatRect( { 0.f, r * region_height }, { region_width, region_height } );
    m_cached_passage_list.push_back( BlockRegion{ new_region, {} } );
    SPDLOG_INFO( "Created cached region {},{} {},{}", new_region.position.x, new_region.position.y, new_region.size.x, new_region.size.y );
  }

  // Do this first to guarantee success otherwise player cannot leave
  connect_end_room_to_nearest_closed_room();

  for ( auto [closed_room_entt, closed_room_cmp] : closed_room_view.each() )
  {
    auto &current_room_cmp = closed_room_cmp;
    for ( auto &direction : directions )
    {
      find_passage_target( current_room_cmp.m_passageconnections[direction], world_area, { closed_room_entt }, OnePassagePerTargetRoom::YES,
                           AllowDuplicatePassages::NO, WalkingType::DRUNK, CachedOnly::TRUE );
    }
  }

  SPDLOG_INFO( "BlockRegion count {}", m_cached_passage_list.size() );

  // tidyPassageBlocks();
}

void PassageSystem::connect_occupied_and_end_room_passages( entt::entity end_room_entt )
{
  std::vector<Cmp::CryptPassageBlock> passage_block_list;
  if ( end_room_entt == entt::null )
  {
    SPDLOG_WARN( "End room entt is null" );
    return;
  }
  auto crypt_end_room_cmp = getReg().try_get<Cmp::CryptRoomEnd>( end_room_entt );
  if ( not crypt_end_room_cmp )
  {
    SPDLOG_WARN( "end room cmp is null" );
    return;
  }

  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::Player::get_position( getReg() ).findIntersection( open_room_cmp ) ) continue;
    auto &occupied_room_cmp = open_room_cmp;

    // no need to search for suitable target, we already have it
    auto current_passage_door = occupied_room_cmp.m_passageconnections[Cmp::CryptPassageDirection::NORTH];
    m_uncached_passage_list = create_drunken_walk( current_passage_door, *crypt_end_room_cmp, { open_room_entt, end_room_entt } );
  }
  tidyPassageBlocks();
  create_uncached_passages();
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
  for ( auto dir : directions )
  {

    find_passage_target( end_room_cmp.m_passageconnections[dir], world_area, { end_room_entt }, OnePassagePerTargetRoom::NO,
                         AllowDuplicatePassages::YES, WalkingType::DRUNK, CachedOnly::TRUE );
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

  if ( m_region_idx >= kMaxRegions )
  {
    m_connect_all_rooms = false;
    open_passages();
    add_spike_traps();
    return;
  }

  auto &[region, blocklist] = m_cached_passage_list[m_region_idx];
  SPDLOG_INFO( "Spawning region {} with {} blocks", m_region_idx, blocklist.size() );
  for ( auto &block : blocklist )
  {
    auto entt = getReg().create();
    getReg().emplace<Cmp::CryptPassageBlock>( entt, block );
  }
  m_region_idx++;
}

/// PRIVATE FUNCTIONS

std::optional<Cmp::CryptPassageBlock> PassageSystem::place_passage_block( unsigned int passage_id, float x, float y,
                                                                          AllowDuplicatePassages duplicates_policy )
{
  Cmp::Position candidate_passage_block_pos_cmp( Utils::snap_to_grid( { x, y }, Utils::Rounding::TOWARDS_ZERO ), Constants::kGridSizePxF );

  // Check if a block already exists at this position
  auto block_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [passage_block_entt, passage_block_cmp] : block_view.each() )
  {
    Cmp::Position existing_passageblock_pos_cmp( passage_block_cmp, Constants::kGridSizePxF );
    if ( ( existing_passageblock_pos_cmp.findIntersection( candidate_passage_block_pos_cmp ) ) and
         ( duplicates_policy == AllowDuplicatePassages::NO ) )
    {
      // SPDLOG_INFO( "CryptPassageBlock already exists at {},{}", x, y );
      return std::nullopt;
    }
  }

  // Allow placement in open rooms (passages connect to rooms)
  // Only block placement if there's a wall collision

  for ( auto &wall_rects : m_cached_wall_components )
  {
    if ( wall_rects.findIntersection( candidate_passage_block_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Wall collision: Cannot place CryptPassageBlock at {},{}", x, y );
      return std::nullopt;
    }
  }

  SPDLOG_DEBUG( "Placed CryptPassageBlock at {},{} (id:{})", x, y, passage_id );

  return Cmp::CryptPassageBlock( candidate_passage_block_pos_cmp.position, passage_id );
};

std::vector<Cmp::CryptPassageBlock> PassageSystem::create_drunken_walk( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                                                                        std::set<entt::entity> exclude_entts,
                                                                        AllowDuplicatePassages duplicates_policy )
{
  Scene::SceneMapSharedPtr crypt_scene_data = m_crypt_scene_data.lock();
  if ( not crypt_scene_data ) std::runtime_error( "Unable to lock Scene::SceneConfigSharedPtr" );
  auto [map_size_grid, map_size_pixel] = crypt_scene_data->map_size();

  int walk_step_count = 0;
  m_current_passage_id++;

  const float kMinSpacing = Constants::kGridSizePxF.x * 2.0f;
  float min_x = std::max( 0.f, std::min( start.x, end_bounds.position.x ) - kMinSpacing );
  float min_y = std::max( 0.f, std::min( start.y, end_bounds.position.y ) - kMinSpacing );
  float max_x = std::min( map_size_pixel.x,
                          std::max( start.x + Constants::kGridSizePxF.x, end_bounds.position.x + end_bounds.size.x ) + kMinSpacing );
  float max_y = std::min( map_size_pixel.y,
                          std::max( start.y + Constants::kGridSizePxF.y, end_bounds.position.y + end_bounds.size.y ) + kMinSpacing );
  sf::FloatRect walk_bounds( sf::Vector2f( min_x, min_y ), sf::Vector2f( max_x - min_x, max_y - min_y ) );

  // cache open room rects once before the walk
  std::vector<sf::FloatRect> open_room_rects;
  open_room_rects.reserve( 32 );
  for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
  {
    if ( exclude_entts.contains( open_room_entt ) ) continue;
    open_room_rects.emplace_back( open_room_cmp.position, open_room_cmp.size );
  }

  // cache existing passage block positions once before the walk
  // NOTE: new blocks added during the walk use m_current_passage_id so we only need OTHER passages cached
  std::vector<sf::Vector2f> other_passage_block_positions;
  other_passage_block_positions.reserve( 256 );
  for ( auto [block_entt, block_cmp] : getReg().view<Cmp::CryptPassageBlock>().each() )
  {
    if ( block_cmp.m_passage_id != m_current_passage_id ) other_passage_block_positions.emplace_back( block_cmp.x, block_cmp.y );
  }

  std::vector<Cmp::CryptPassageBlock> passage_block_list;

  auto maybe_passage_block = place_passage_block( m_current_passage_id, start.x, start.y, duplicates_policy );
  if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }

  sf::FloatRect current_pos( { start.x, start.y }, Constants::kGridSizePxF );
  sf::Vector2f last_move_direction( 0.f, 0.f );

  std::vector<sf::Vector2f> recent_positions;
  const size_t max_recent_positions = 5;

  while ( not current_pos.findIntersection( end_bounds ) && walk_step_count < kMaxStepsPerWalk )
  {
    sf::FloatRect candidate_pos( { -16.f, -16.f }, Constants::kGridSizePxF );
    int step_attempts = 0;
    bool is_candidate_rejected = false;
    bool found_valid_candidate = false;

    do
    {
      sf::Vector2f target_distance = end_bounds.getCenter() - current_pos.getCenter();
      sf::Vector2f chosen_direction;
      int random_choice = m_direction_picker.gen();

      if ( walk_step_count < kMinInitialOrthogonalSteps ) { chosen_direction = m_direction_dictionary[start.m_direction]; }
      else
      {
        if ( random_choice < static_cast<int>( m_roulette_target_bias_odds * 100 ) )
        {
          if ( std::abs( target_distance.x ) > std::abs( target_distance.y ) )
            chosen_direction = ( target_distance.x > 0 ) ? sf::Vector2f( 1.f, 0.f ) : sf::Vector2f( -1.f, 0.f );
          else
            chosen_direction = ( target_distance.y > 0 ) ? sf::Vector2f( 0.f, 1.f ) : sf::Vector2f( 0.f, -1.f );
        }
        else if ( random_choice < static_cast<int>( ( m_roulette_target_bias_odds + m_roulette_same_direction_odds ) * 100 ) &&
                  ( last_move_direction.x != 0.f || last_move_direction.y != 0.f ) )
        {
          chosen_direction = last_move_direction;
        }
        else
        {
          auto random_dir_idx = Cmp::RandomInt( 0, m_direction_choices.size() - 1 );
          chosen_direction = m_direction_choices[random_dir_idx.gen()];
        }
      }

      auto chosen_magnitude = chosen_direction.componentWiseMul( Constants::kGridSizePxF );
      candidate_pos.position.x = current_pos.position.x + chosen_magnitude.x;
      candidate_pos.position.y = current_pos.position.y + chosen_magnitude.y;

      if ( !walk_bounds.contains( candidate_pos.position ) ||
           candidate_pos.position.x + candidate_pos.size.x > walk_bounds.position.x + walk_bounds.size.x ||
           candidate_pos.position.y + candidate_pos.size.y > walk_bounds.position.y + walk_bounds.size.y )
      {
        step_attempts++;
        continue;
      }

      bool is_recent = false;
      for ( const auto &recent_pos : recent_positions )
      {
        if ( std::abs( recent_pos.x - candidate_pos.position.x ) < Constants::kGridSizePxF.x * 0.5f &&
             std::abs( recent_pos.y - candidate_pos.position.y ) < Constants::kGridSizePxF.y * 0.5f )
        {
          is_recent = true;
          break;
        }
      }
      if ( is_recent && step_attempts < kMaxAttemptsPerStep * 0.7f )
      {
        step_attempts++;
        continue;
      }

      is_candidate_rejected = false;

      if ( duplicates_policy == AllowDuplicatePassages::NO )
      {
        // use cached other_passage_block_positions instead of registry view
        for ( const auto &block_pos : other_passage_block_positions )
        {
          float distance = Utils::Maths::getEuclideanDistance( candidate_pos.position, block_pos );
          if ( distance < kMinBlockDistanceBetweenPassages )
          {
            is_candidate_rejected = true;
            break;
          }
        }

        // use cached open_room_rects instead of registry view
        if ( !is_candidate_rejected && walk_step_count > kMinPassageRoomsDistanceDelay )
        {
          Cmp::RectBounds scaled_candidate_pos( candidate_pos.position, candidate_pos.size, kMinPassageRoomsDistanceScaleFactor );
          for ( const auto &room_rect : open_room_rects )
          {
            if ( scaled_candidate_pos.findIntersection( room_rect ) )
            {
              is_candidate_rejected = true;
              break;
            }
          }
        }
      }

      // use cached wall_rects instead of registry view
      if ( !is_candidate_rejected )
      {
        for ( const auto &wall_rect : m_cached_wall_components )
        {
          if ( candidate_pos.findIntersection( wall_rect ) )
          {
            is_candidate_rejected = true;
            break;
          }
        }
      }

      if ( !is_candidate_rejected )
      {
        found_valid_candidate = true;
        break;
      }
      step_attempts++;

    } while ( step_attempts < kMaxAttemptsPerStep );

    if ( !found_valid_candidate )
    {
      SPDLOG_DEBUG( "createDrunkenWalkPassage (id:{}): No valid candidate found after {} attempts, terminating walk", m_current_passage_id,
                    kMaxAttemptsPerStep );
      return {};
    }

    sf::Vector2f old_pos = current_pos.position;
    current_pos.position = candidate_pos.position;
    last_move_direction = sf::Vector2f( ( current_pos.position.x - old_pos.x ) / Constants::kGridSizePxF.x,
                                        ( current_pos.position.y - old_pos.y ) / Constants::kGridSizePxF.y );

    recent_positions.push_back( current_pos.position );
    if ( recent_positions.size() > max_recent_positions ) { recent_positions.erase( recent_positions.begin() ); }

    maybe_passage_block = place_passage_block( m_current_passage_id, current_pos.position.x, current_pos.position.y, duplicates_policy );
    if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }

    walk_step_count++;
  }

  if ( walk_step_count >= kMaxStepsPerWalk )
  {
    SPDLOG_WARN( "createDrunkenWalkPassage (id:{}): Maximum walk length reached, terminating", m_current_passage_id );
    return {};
  }

  return passage_block_list;
}

void PassageSystem::find_passage_target( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant,
                                         std::set<entt::entity> exclude_entts, OnePassagePerTargetRoom passage_limit,
                                         AllowDuplicatePassages duplicates_policy, WalkingType walktype, CachedOnly cached )
{

  std::vector<Cmp::CryptPassageBlock> passage_list;

  // clang-format off
  using MidPointDistanceQueue = std::priority_queue < 
    std::pair<float, entt::entity>,
    std::vector<std::pair<float, entt::entity>>, 
    std::greater<std::pair<float, entt::entity>>
  >;
  // clang-format on

  // Assemble list of distances to other rooms and sort it by nearest first
  auto get_sorted_distances = [&]( auto &&room_view ) -> MidPointDistanceQueue
  {
    MidPointDistanceQueue pqueue;
    for ( auto [other_room_entt, other_room_cmp] : room_view.each() )
    {
      if ( exclude_entts.contains( other_room_entt ) ) continue;
      if ( not other_room_cmp.findIntersection( search_quadrant ) ) continue;
      if ( other_room_cmp.are_all_doors_used() ) continue;
      auto distance = Utils::Maths::getEuclideanDistance( start_passage_door, other_room_cmp.getCenter() );
      SPDLOG_DEBUG( "Found room {} at distance {}", static_cast<uint32_t>( other_room_entt ), distance );
      pqueue.push( { distance, other_room_entt } );
    }
    return pqueue;
  };

  MidPointDistanceQueue dist_pqueue = ( cached == CachedOnly::TRUE ) ? get_sorted_distances( getReg().view<Cmp::CryptRoomClosed>() )
                                                                     : get_sorted_distances( getReg().view<Cmp::CryptRoomOpen>() );

  auto get_bounds = [&]( entt::entity entt ) -> std::optional<sf::FloatRect>
  {
    auto *cmp = ( cached == CachedOnly::TRUE ) ? static_cast<sf::FloatRect *>( getReg().try_get<Cmp::CryptRoomClosed>( entt ) )
                                               : static_cast<sf::FloatRect *>( getReg().try_get<Cmp::CryptRoomOpen>( entt ) );
    if ( !cmp )
    {
      SPDLOG_WARN( "find_passage_target: entity {} missing room component, skipping", static_cast<uint32_t>( entt ) );
      return std::nullopt;
    }
    return sf::FloatRect( cmp->position, cmp->size );
  };

  // process the distance list, one room at a time
  while ( not dist_pqueue.empty() )
  {
    auto nearest_other_room_entt = dist_pqueue.top().second;
    dist_pqueue.pop();

    if ( not getReg().valid( nearest_other_room_entt ) ) continue;

    auto maybe_bounds = get_bounds( nearest_other_room_entt );
    if ( not maybe_bounds ) continue;

    // try to create a room-to-room pathway
    if ( walktype == WalkingType::DRUNK )
    {
      passage_list = create_drunken_walk( start_passage_door, maybe_bounds.value(), { nearest_other_room_entt }, duplicates_policy );
    }
    else { passage_list = create_dog_leg( start_passage_door, maybe_bounds.value(), duplicates_policy ); }

    // if the pathway was successfull in reaching goal
    if ( not passage_list.empty() )
    {

      auto insert_passage = [&]( auto *room_cmp, auto &passage_store )
      {
        if ( passage_limit == OnePassagePerTargetRoom::YES && room_cmp ) { room_cmp->set_all_doors_used( true ); }
        passage_store.insert( passage_store.begin(), passage_list.begin(), passage_list.end() );
      };

      if ( cached == CachedOnly::FALSE )
      {
        start_passage_door.is_used = true;
        insert_passage( getReg().try_get<Cmp::CryptRoomOpen>( nearest_other_room_entt ), m_uncached_passage_list );
      }
      else
      {
        for ( auto &passage_block_cmp : passage_list )
        {
          for ( auto &[region, blocklist] : m_cached_passage_list )
          {
            if ( not sf::FloatRect( passage_block_cmp, Constants::kGridSizePxF ).findIntersection( region ) ) continue;
            auto room_cmp = getReg().try_get<Cmp::CryptRoomClosed>( nearest_other_room_entt );
            if ( passage_limit == OnePassagePerTargetRoom::YES && room_cmp ) { room_cmp->set_all_doors_used( true ); }
            blocklist.push_back( passage_block_cmp );
          }
        }
      }

      break;
    }
  }
}

void PassageSystem::removeAllPassageBlocks()
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

void PassageSystem::emptyOpenPassages()
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

void PassageSystem::fillAllPassages()
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

void PassageSystem::tidyPassageBlocks( bool include_closed_rooms )
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

std::vector<Cmp::CryptPassageBlock> PassageSystem::create_dog_leg( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                                                                   AllowDuplicatePassages duplicates_policy )
{

  std::vector<Cmp::CryptPassageBlock> passage_block_list;
  m_current_passage_id++;
  SPDLOG_DEBUG( "createDogLegPassage (id:{}) from ({},{}) to ({},{})", m_current_passage_id, start.x, start.y, end_bounds.getCenter().x,
                end_bounds.getCenter().y );

  const auto kSquareSizePx = Constants::kGridSizePxF;

  // always prioritize nearer direction
  if ( start.m_direction == Cmp::CryptPassageDirection::EAST or start.m_direction == Cmp::CryptPassageDirection::WEST )
  {
    SPDLOG_DEBUG( "First leg: horizontal" );
    // try to prevent hitting another open room before reaching the target room
    float furthest_pos_x;
    if ( start.m_direction == Cmp::CryptPassageDirection::EAST )
    {
      furthest_pos_x = end_bounds.position.x + end_bounds.size.x;
      for ( float x = start.x; x <= furthest_pos_x; x += kSquareSizePx.x )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, x, start.y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    else if ( start.m_direction == Cmp::CryptPassageDirection::WEST )
    {
      furthest_pos_x = end_bounds.position.x;
      for ( float x = start.x; x >= furthest_pos_x; x -= kSquareSizePx.x )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, x, start.y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    SPDLOG_DEBUG( "Second leg: vertical from end.x to end.y" );
    float dy = end_bounds.getCenter().y - start.y;
    if ( dy > 0 )
    {
      for ( float y = start.y + kSquareSizePx.y; y <= end_bounds.getCenter().y; y += kSquareSizePx.y )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, furthest_pos_x, y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    else
    {
      for ( float y = start.y - kSquareSizePx.y; y >= end_bounds.getCenter().y; y -= kSquareSizePx.y )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, furthest_pos_x, y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
  }
  else
  {
    SPDLOG_DEBUG( "First leg: vertical" );
    // try to prevent hitting another open room before reaching the target room
    float furthest_pos_y = 0;
    if ( start.m_direction == Cmp::CryptPassageDirection::SOUTH )
    {
      furthest_pos_y = end_bounds.position.y + end_bounds.size.y;
      for ( float y = start.y; y <= furthest_pos_y; y += kSquareSizePx.y )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, start.x, y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    else if ( start.m_direction == Cmp::CryptPassageDirection::NORTH )
    {
      furthest_pos_y = end_bounds.position.y;
      for ( float y = start.y; y >= furthest_pos_y; y -= kSquareSizePx.y )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, start.x, y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    SPDLOG_DEBUG( "Second leg: horizontal from start.x to end.x" );
    float dx = end_bounds.getCenter().x - start.x;
    if ( dx > 0 )
    {
      for ( float x = start.x + kSquareSizePx.x; x <= end_bounds.getCenter().x; x += kSquareSizePx.x )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, x, furthest_pos_y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
    else
    {
      for ( float x = start.x - kSquareSizePx.x; x >= end_bounds.getCenter().x; x -= kSquareSizePx.x )
      {
        auto maybe_passage_block = place_passage_block( m_current_passage_id, x, furthest_pos_y, duplicates_policy );
        if ( maybe_passage_block.has_value() ) { passage_block_list.push_back( maybe_passage_block.value() ); }
      }
    }
  }

  return passage_block_list;
}

} // namespace ProceduralMaze::Sys