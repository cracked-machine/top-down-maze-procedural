#include <Components/CryptPassageBlock.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Wall.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/PassageSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Utils.hpp>

#include <queue>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

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
      connectPassagesBetweenStartAndOpenRooms( event.entity );
      break;
    case Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_OPENROOMS:
      connectPassagesBetweenOccupiedAndOpenRooms();
      break;
    case Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_ENDROOM:
      connectPassageBetweenOccupiedAndEndRoom( event.entity );
      break;
    case Events::PassageEvent::Type::CONNECT_ALL_OPENROOMS:
      connectPassagesBetweenAllOpenRooms();
      break;
  }
}

void PassageSystem::remove_passages()
{
  fillAllPassages();
  removeAllPassageBlocks();
}

void PassageSystem::open_passages()
{
  //
  emptyOpenPassages();
}

void PassageSystem::connectPassagesBetweenStartAndOpenRooms( entt::entity start_room_entt )
{
  if ( start_room_entt == entt::null )
  {
    SPDLOG_WARN( "PassageSystem::connectPassagesBetweenStartAndOpenRooms - start_room_entt is null" );
    return;
  }
  auto start_room_cmp = getReg().try_get<Cmp::CryptRoomStart>( start_room_entt );
  if ( not start_room_cmp )
  {
    SPDLOG_WARN( "PassageSystem::connectPassagesBetweenStartAndOpenRooms - start_room_cmp is null" );
    return;
  }

  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );
  const auto start_room_right_pos_x = start_room_cmp->position.x + start_room_cmp->size.x;

  // divide the gamrarea into 3 quadrants - again there are only three because startroom is southern most position in the game area
  auto west_quad = sf::FloatRect( { 0.f, 0.f }, { start_room_cmp->position.x, world_size.y } );
  SPDLOG_INFO( "west_quadrant: {},{} : {},{}", west_quad.position.x, west_quad.position.y, west_quad.size.x, west_quad.size.y );

  auto east_quad = sf::FloatRect( { start_room_right_pos_x, 0.f }, { world_size.x - ( start_room_right_pos_x ), world_size.y } );
  SPDLOG_INFO( "east_quadrant: {},{} : {},{}", east_quad.position.x, east_quad.position.y, east_quad.size.x, east_quad.size.y );

  auto north_quad = sf::FloatRect( { 0.f, 0.f }, { world_size.x, start_room_cmp->position.y } );
  SPDLOG_INFO( "north_quadrant: {},{} : {},{}", north_quad.position.x, north_quad.position.y, north_quad.size.x, north_quad.size.y );

  find_passage_target( start_room_cmp->m_midpoints[Cmp::CryptPassageDirection::WEST], west_quad, { start_room_entt } );
  find_passage_target( start_room_cmp->m_midpoints[Cmp::CryptPassageDirection::EAST], east_quad, { start_room_entt } );
  find_passage_target( start_room_cmp->m_midpoints[Cmp::CryptPassageDirection::NORTH], north_quad, { start_room_entt } );
  tidyPassageBlocks();
}

void PassageSystem::connectPassagesBetweenOccupiedAndOpenRooms()
{
  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );

  // find the open room that the player is in (if any)
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::get_player_position( getReg() ).findIntersection( open_room_cmp ) ) continue;

    auto &occupied_room_cmp = open_room_cmp;

    const auto current_room_right_pos_x = occupied_room_cmp.position.x + occupied_room_cmp.size.x;
    const auto current_room_bottom_pos_y = occupied_room_cmp.position.y + occupied_room_cmp.size.y;

    // divide the gamrarea into 4 quadrants
    auto west_quad = sf::FloatRect( { 0.f, 0.f }, { occupied_room_cmp.position.x, world_size.y } );
    SPDLOG_INFO( "west_quadrant: {},{} : {},{}", west_quad.position.x, west_quad.position.y, west_quad.size.x, west_quad.size.y );

    auto east_quad = sf::FloatRect( { current_room_right_pos_x, 0.f }, { world_size.x - ( current_room_right_pos_x ), world_size.y } );
    SPDLOG_INFO( "east_quadrant: {},{} : {},{}", east_quad.position.x, east_quad.position.y, east_quad.size.x, east_quad.size.y );

    auto north_quad = sf::FloatRect( { 0.f, 0.f }, { world_size.x, occupied_room_cmp.position.y } );
    SPDLOG_INFO( "north_quadrant: {},{} : {},{}", north_quad.position.x, north_quad.position.y, north_quad.size.x, north_quad.size.y );

    auto south_quad = sf::FloatRect( { occupied_room_cmp.position.x, current_room_bottom_pos_y },
                                     { current_room_right_pos_x, world_size.y - ( current_room_bottom_pos_y ) } );
    SPDLOG_INFO( "south_quadrant: {},{} : {},{}", south_quad.position.x, south_quad.position.y, south_quad.size.x, south_quad.size.y );

    find_passage_target( occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::WEST], west_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::EAST], east_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH], north_quad, { open_room_entt } );
    find_passage_target( occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::SOUTH], south_quad, { open_room_entt } );

    tidyPassageBlocks();
  }
}

void PassageSystem::connectPassagesBetweenAllOpenRooms()
{
  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );
  const auto world_area = sf::FloatRect( { 0, 0 }, world_size );

  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    auto &current_room_cmp = open_room_cmp;
    find_passage_target( current_room_cmp.m_midpoints[Cmp::CryptPassageDirection::WEST], world_area, { open_room_entt }, OnePassagePerTargetRoom::NO,
                         AllowDuplicatePassages::YES );
    find_passage_target( current_room_cmp.m_midpoints[Cmp::CryptPassageDirection::EAST], world_area, { open_room_entt }, OnePassagePerTargetRoom::NO,
                         AllowDuplicatePassages::YES );
    find_passage_target( current_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH], world_area, { open_room_entt }, OnePassagePerTargetRoom::NO,
                         AllowDuplicatePassages::YES );
    find_passage_target( current_room_cmp.m_midpoints[Cmp::CryptPassageDirection::SOUTH], world_area, { open_room_entt }, OnePassagePerTargetRoom::NO,
                         AllowDuplicatePassages::YES );
    current_room_cmp.set_all_doors_used( true );
  }

  tidyPassageBlocks();
}

void PassageSystem::connectPassageBetweenOccupiedAndEndRoom( entt::entity end_room_entt )
{
  if ( end_room_entt == entt::null )
  {
    SPDLOG_WARN( "PassageSystem::connectPassageBetweenOccupiedAndEndRoom - end_room_entt is null" );
    return;
  }
  auto crypt_end_room_cmp = getReg().try_get<Cmp::CryptRoomEnd>( end_room_entt );
  if ( not crypt_end_room_cmp )
  {
    SPDLOG_WARN( "PassageSystem::connectPassageBetweenOccupiedAndEndRoom - crypt_end_room_cmp is null" );
    return;
  }

  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::get_player_position( getReg() ).findIntersection( open_room_cmp ) ) continue;
    auto &occupied_room_cmp = open_room_cmp;

    // no need to search for suitable target, we already have it
    auto current_passage_door = occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH];
    createDrunkenWalkPassage( current_passage_door, *crypt_end_room_cmp, { open_room_entt, end_room_entt } );
  }

  tidyPassageBlocks();
}

/// PRIVATE FUNCTIONS

bool PassageSystem::place_passage_block( unsigned int passage_id, float x, float y, AllowDuplicatePassages duplicates_policy )
{
  // track additions so we can roll them back if a violation is found
  std::vector<entt::entity> new_block_list;

  auto block_list_unwind = [&]()
  {
    for ( auto &passage_block_entity : new_block_list )
    {
      getReg().destroy( passage_block_entity );
    }
    new_block_list.clear();
  };

  Cmp::Position next_passage_block_cmp( Utils::snap_to_grid( { x, y }, Utils::Rounding::TOWARDS_ZERO ), Constants::kGridSquareSizePixelsF );

  // Check if a block already exists at this position
  auto block_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [passage_block_entt, passage_block_cmp] : block_view.each() )
  {
    Cmp::Position found_passage_block_pos_cmp( passage_block_cmp, Constants::kGridSquareSizePixelsF );
    if ( ( found_passage_block_pos_cmp.findIntersection( next_passage_block_cmp ) ) and ( duplicates_policy == AllowDuplicatePassages::NO ) )
    {
      SPDLOG_INFO( "CryptPassageBlock already exists at {},{}", x, y );
      block_list_unwind();
      return false;
    }
  }

  // Allow placement in open rooms (passages connect to rooms)
  // Only block placement if there's a wall collision
  auto wall_view = getReg().view<Cmp::Wall, Cmp::Position>();
  for ( auto [wall_entt, wall_cmp, wall_pos_cmp] : wall_view.each() )
  {
    if ( wall_pos_cmp.findIntersection( next_passage_block_cmp ) )
    {
      SPDLOG_INFO( "Wall collision: Cannot place CryptPassageBlock at {},{}", x, y );
      block_list_unwind();
      return false;
    }
  }

  entt::entity new_entt = getReg().create();
  getReg().emplace_or_replace<Cmp::CryptPassageBlock>( new_entt, next_passage_block_cmp.position, passage_id );
  new_block_list.push_back( new_entt );
  SPDLOG_DEBUG( "Placed CryptPassageBlock at {},{} (id:{})", x, y, passage_id );

  return true;
};

bool PassageSystem::createDogLegPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, AllowDuplicatePassages duplicates_policy )
{

  m_current_passage_id++;
  SPDLOG_INFO( "createDogLegPassage (id:{}) from ({},{}) to ({},{})", m_current_passage_id, start.x, start.y, end_bounds.getCenter().x,
               end_bounds.getCenter().y );

  const auto kSquareSizePx = Constants::kGridSquareSizePixelsF;

  // always prioritize nearer direction
  if ( start.m_direction == Cmp::CryptPassageDirection::EAST or start.m_direction == Cmp::CryptPassageDirection::WEST )
  {
    SPDLOG_INFO( "First leg: horizontal" );
    // try to prevent hitting another open room before reaching the target room
    float furthest_pos_x;
    if ( start.m_direction == Cmp::CryptPassageDirection::EAST )
    {
      furthest_pos_x = end_bounds.position.x + end_bounds.size.x;
      for ( float x = start.x; x <= furthest_pos_x; x += kSquareSizePx.x )
      {
        if ( not place_passage_block( m_current_passage_id, x, start.y, duplicates_policy ) ) { return false; }
      }
    }
    else if ( start.m_direction == Cmp::CryptPassageDirection::WEST )
    {
      furthest_pos_x = end_bounds.position.x;
      for ( float x = start.x; x >= furthest_pos_x; x -= kSquareSizePx.x )
      {
        if ( not place_passage_block( m_current_passage_id, x, start.y, duplicates_policy ) ) { return false; }
      }
    }
    SPDLOG_INFO( "Second leg: vertical from end.x to end.y" );
    float dy = end_bounds.getCenter().y - start.y;
    if ( dy > 0 )
    {
      for ( float y = start.y + kSquareSizePx.y; y <= end_bounds.getCenter().y; y += kSquareSizePx.y )
      {
        if ( not place_passage_block( m_current_passage_id, furthest_pos_x, y, duplicates_policy ) ) { return false; }
      }
    }
    else
    {
      for ( float y = start.y - kSquareSizePx.y; y >= end_bounds.getCenter().y; y -= kSquareSizePx.y )
      {
        if ( not place_passage_block( m_current_passage_id, furthest_pos_x, y, duplicates_policy ) ) { return false; }
      }
    }
  }
  else
  {
    SPDLOG_INFO( "First leg: vertical" );
    // try to prevent hitting another open room before reaching the target room
    float furthest_pos_y;
    if ( start.m_direction == Cmp::CryptPassageDirection::SOUTH )
    {
      furthest_pos_y = end_bounds.position.y + end_bounds.size.y;
      for ( float y = start.y; y <= furthest_pos_y; y += kSquareSizePx.y )
      {
        if ( not place_passage_block( m_current_passage_id, start.x, y, duplicates_policy ) ) { return false; }
      }
    }
    else if ( start.m_direction == Cmp::CryptPassageDirection::NORTH )
    {
      furthest_pos_y = end_bounds.position.y;
      for ( float y = start.y; y >= furthest_pos_y; y -= kSquareSizePx.y )
      {
        if ( not place_passage_block( m_current_passage_id, start.x, y, duplicates_policy ) ) { return false; }
      }
    }
    SPDLOG_INFO( "Second leg: horizontal from start.x to end.x" );
    float dx = end_bounds.getCenter().x - start.x;
    if ( dx > 0 )
    {
      for ( float x = start.x + kSquareSizePx.x; x <= end_bounds.getCenter().x; x += kSquareSizePx.x )
      {
        if ( not place_passage_block( m_current_passage_id, x, furthest_pos_y, duplicates_policy ) ) { return false; }
      }
    }
    else
    {
      for ( float x = start.x - kSquareSizePx.x; x >= end_bounds.getCenter().x; x -= kSquareSizePx.x )
      {
        if ( not place_passage_block( m_current_passage_id, x, furthest_pos_y, duplicates_policy ) ) { return false; }
      }
    }
  }

  return true;
}

bool PassageSystem::createDrunkenWalkPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, std::set<entt::entity> exclude_entts,
                                              AllowDuplicatePassages duplicates_policy )
{
  // Generate unique walk ID
  m_current_passage_id++;
  SPDLOG_INFO( "createDrunkenWalkPassage (id:{}) from ({},{}) to ({},{})", m_current_passage_id, start.x, start.y, end_bounds.getCenter().x,
               end_bounds.getCenter().y );

  int walk_step_count = 0;

  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );

  // Calculate expanded walk_bounds with some padding
  float padding = Constants::kGridSquareSizePixelsF.x * 2.0f;
  float min_x = std::min( start.x, end_bounds.position.x ) - padding;
  float min_y = std::min( start.y, end_bounds.position.y ) - padding;
  float max_x = std::max( start.x + Constants::kGridSquareSizePixelsF.x, end_bounds.position.x + end_bounds.size.x ) + padding;
  float max_y = std::max( start.y + Constants::kGridSquareSizePixelsF.y, end_bounds.position.y + end_bounds.size.y ) + padding;

  // Clamp to world bounds
  min_x = std::max( 0.f, min_x );
  min_y = std::max( 0.f, min_y );
  max_x = std::min( world_size.x, max_x );
  max_y = std::min( world_size.y, max_y );

  sf::FloatRect walk_bounds( sf::Vector2f( min_x, min_y ), sf::Vector2f( max_x - min_x, max_y - min_y ) );

  SPDLOG_INFO( "walk_bounds = {},{} to {},{}", walk_bounds.position.x, walk_bounds.position.y, walk_bounds.position.x + walk_bounds.size.x,
               walk_bounds.position.y + walk_bounds.size.y );

  place_passage_block( m_current_passage_id, start.x, start.y, duplicates_policy );
  sf::FloatRect current_pos( { start.x, start.y }, Constants::kGridSquareSizePixelsF );

  sf::Vector2f last_move_direction( 0.f, 0.f );

  // Track recent positions to avoid immediate backtracking
  std::vector<sf::Vector2f> recent_positions;
  const size_t max_recent_positions = 5;

  // keep walking until we reach our target or hit limits
  while ( not current_pos.findIntersection( end_bounds ) && walk_step_count < kMaxStepsPerWalk )
  {
    sf::FloatRect candidate_pos( { -16.f, -16.f }, Constants::kGridSquareSizePixelsF );
    int step_attempts = 0;
    bool is_candidate_rejected = false;
    bool found_valid_candidate = false;

    do
    {
      // Calculate direction towards target
      sf::Vector2f to_target = end_bounds.getCenter() - current_pos.getCenter();

      sf::Vector2f chosen_direction;
      int random_choice = m_direction_picker.gen();

      // Force initial steps to go in orthogonal direction from start
      if ( walk_step_count < kMinInitialOrthogonalSteps ) { chosen_direction = m_direction_dictionary[start.m_direction]; }
      // Roulette time!
      else
      {
        // Bias toward target: choose direction that reduces distance to target
        if ( random_choice < static_cast<int>( m_roulette_target_bias_odds * 100 ) )
        {
          if ( std::abs( to_target.x ) > std::abs( to_target.y ) )
          {
            chosen_direction = ( to_target.x > 0 ) ? sf::Vector2f( 1.f, 0.f ) : sf::Vector2f( -1.f, 0.f );
          }
          else { chosen_direction = ( to_target.y > 0 ) ? sf::Vector2f( 0.f, 1.f ) : sf::Vector2f( 0.f, -1.f ); }
        }
        // Continue in same direction
        else if ( random_choice < static_cast<int>( ( m_roulette_target_bias_odds + m_roulette_same_direction_odds ) * 100 ) &&
                  ( last_move_direction.x != 0.f || last_move_direction.y != 0.f ) )
        {
          chosen_direction = last_move_direction;
        }
        // Random direction
        else
        {
          auto random_dir_idx = Cmp::RandomInt( 0, m_direction_choices.size() - 1 );
          chosen_direction = m_direction_choices[random_dir_idx.gen()];
        }
      }

      // Prepare the actual movement and run some final rule validation
      auto chosen_magnitude = chosen_direction.componentWiseMul( Constants::kGridSquareSizePixelsF );
      candidate_pos.position.x = current_pos.position.x + chosen_magnitude.x;
      candidate_pos.position.y = current_pos.position.y + chosen_magnitude.y;

      // Check if candidate is within walking bounds
      if ( !walk_bounds.contains( candidate_pos.position ) ||
           candidate_pos.position.x + candidate_pos.size.x > walk_bounds.position.x + walk_bounds.size.x ||
           candidate_pos.position.y + candidate_pos.size.y > walk_bounds.position.y + walk_bounds.size.y )
      {
        step_attempts++;
        continue;
      }

      // Check if candidate was recently visited (avoid immediate backtracking)
      bool is_recent = false;
      for ( const auto &recent_pos : recent_positions )
      {
        if ( std::abs( recent_pos.x - candidate_pos.position.x ) < Constants::kGridSquareSizePixelsF.x * 0.5f &&
             std::abs( recent_pos.y - candidate_pos.position.y ) < Constants::kGridSquareSizePixelsF.y * 0.5f )
        {
          is_recent = true;
          break;
        }
      }

      if ( is_recent && step_attempts < kMaxAttemptsPerStep * 0.7f ) // Allow recent positions if we're running out of attempts
      {
        step_attempts++;
        continue;
      }

      // Check constraints
      is_candidate_rejected = false;

      // Check passage-to-passage distance
      if ( duplicates_policy == AllowDuplicatePassages::NO )
      {
        for ( auto [block_entt, block_cmp] : getReg().view<Cmp::CryptPassageBlock>().each() )
        {
          if ( block_cmp.m_passage_id != m_current_passage_id )
          {
            float distance = Utils::Maths::getEuclideanDistance( candidate_pos.position, block_cmp );
            if ( distance < kMinBlockDistanceBetweenPassages )
            {
              is_candidate_rejected = true;
              SPDLOG_DEBUG( "Candidate rejected for passage id :{} - PassageBlock {},{} too close to candidate {},{}", m_current_passage_id,
                            block_cmp.x, block_cmp.y, candidate_pos.position.x, candidate_pos.position.y );
              break;
            }
          }
        }

        // Check passage-to-room distance
        if ( !is_candidate_rejected )
        {
          for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
          {
            if ( exclude_entts.contains( open_room_entt ) ) continue;
            if ( walk_step_count > kMinPassageRoomsDistanceDelay )
            {
              Cmp::RectBounds scaled_candidate_pos( candidate_pos.position, candidate_pos.size, kMinPassageRoomsDistanceScaleFactor );
              if ( scaled_candidate_pos.findIntersection( open_room_cmp ) )
              {
                is_candidate_rejected = true;
                SPDLOG_DEBUG( "Candidate rejected for passage id :{} - Open room {},{} too close to candidate {},{}", m_current_passage_id,
                              open_room_cmp.position.x, open_room_cmp.position.y, candidate_pos.position.x, candidate_pos.position.y );
                break;
              }
            }
          }
        }
      }

      // Check passage-to-wall distance
      if ( !is_candidate_rejected )
      {
        for ( auto [wall_entt, wall_cmp, wall_pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
        {
          if ( candidate_pos.findIntersection( wall_pos_cmp ) )
          {
            is_candidate_rejected = true;
            SPDLOG_DEBUG( "Candidate rejected for passage id :{} - Wall {},{} too close to candidate {},{}", m_current_passage_id,
                          wall_pos_cmp.position.x, wall_pos_cmp.position.y, candidate_pos.position.x, candidate_pos.position.y );
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
      SPDLOG_WARN( "createDrunkenWalkPassage (id:{}): No valid candidate found after {} attempts, terminating walk", m_current_passage_id,
                   kMaxAttemptsPerStep );
      return false;
    }

    // Update position and remember the direction we moved
    sf::Vector2f old_pos = current_pos.position;
    current_pos.position = candidate_pos.position;
    last_move_direction = sf::Vector2f( ( current_pos.position.x - old_pos.x ) / Constants::kGridSquareSizePixelsF.x,
                                        ( current_pos.position.y - old_pos.y ) / Constants::kGridSquareSizePixelsF.y );

    // Track recent positions
    recent_positions.push_back( current_pos.position );
    if ( recent_positions.size() > max_recent_positions ) { recent_positions.erase( recent_positions.begin() ); }

    place_passage_block( m_current_passage_id, current_pos.position.x, current_pos.position.y, duplicates_policy );
    walk_step_count++;
  }

  if ( walk_step_count >= kMaxStepsPerWalk )
  {
    SPDLOG_WARN( "createDrunkenWalkPassage (id:{}): Maximum walk length reached, terminating", m_current_passage_id );
    return false;
  }

  SPDLOG_INFO( "++++++++++++++++++++ Target Reached (id:{}) +++++++++++++++", m_current_passage_id );
  return true;
}

void PassageSystem::find_passage_target( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant,
                                         std::set<entt::entity> exclude_entts, OnePassagePerTargetRoom passage_limit,
                                         AllowDuplicatePassages duplicates_policy )
{
  // clang-format off
  using MidPointDistanceQueue = std::priority_queue < 
    std::pair<float, entt::entity>,
    std::vector<std::pair<float, entt::entity>>, 
    std::greater<std::pair<float, entt::entity>>
  >;
  // clang-format on

  MidPointDistanceQueue dist_pqueue;
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [other_room_entt, other_room_cmp] : open_room_view.each() )
  {
    if ( exclude_entts.contains( other_room_entt ) ) continue;
    if ( not other_room_cmp.findIntersection( search_quadrant ) ) continue;
    if ( other_room_cmp.are_all_doors_used() ) continue;
    dist_pqueue.push( { Utils::Maths::getEuclideanDistance( start_passage_door, other_room_cmp.getCenter() ), other_room_entt } );
  }

  SPDLOG_INFO( "north_dist_pqueue - size:{}", dist_pqueue.size() );
  if ( not dist_pqueue.empty() )
  {
    auto nearest_other_room_entt = dist_pqueue.top().second;
    auto &nearest_other_room_cmp = getReg().get<Cmp::CryptRoomOpen>( nearest_other_room_entt );
    if ( createDrunkenWalkPassage( start_passage_door, nearest_other_room_cmp, { nearest_other_room_entt }, duplicates_policy ) )
    {
      start_passage_door.is_used = true;
      if ( passage_limit == OnePassagePerTargetRoom::YES )
      {
        nearest_other_room_cmp.set_all_doors_used( true ); // prevent more passages
      }
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
  auto crypt_passage_block_view_remaining = getReg().view<Cmp::CryptPassageBlock>();
  SPDLOG_INFO( "Remaining Cmp::CryptPassageBlock entities: {}", crypt_passage_block_view_remaining.size() );
}

void PassageSystem::emptyOpenPassages()
{
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    auto pblock_view = getReg().view<Cmp::CryptPassageBlock>();
    for ( auto [pblock_entt, pblock_cmp] : pblock_view.each() )
    {
      auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSquareSizePixelsF );
      // skip any positions that are not pblocks or do not have obstacles
      if ( not pblock_cmp_rect.findIntersection( pos_cmp ) ) continue;
      if ( not getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      Factory::destroyObstacle( getReg(), pos_entt );
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
      auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSquareSizePixelsF );
      // skip any positions that are not pblocks or already have obstacles
      if ( not pblock_cmp_rect.findIntersection( pos_cmp ) ) continue;
      if ( getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      Factory::createObstacle( getReg(), pos_entt, pos_cmp, obst_type, 2, ( zorder * 2.f ) );

      if ( Utils::getSystemCmp( getReg() ).collisions_enabled )
      {
        if ( Utils::get_player_position( getReg() ).findIntersection( pblock_cmp_rect ) )
        {
          // player got squished
          get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SQUISHED ) );
        }
      }
    }
  }
}

void PassageSystem::tidyPassageBlocks( bool include_closed_rooms )
{
  for ( auto [pblock_entt, pblock_cmp] : getReg().view<Cmp::CryptPassageBlock>().each() )
  {
    auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSquareSizePixelsF );

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