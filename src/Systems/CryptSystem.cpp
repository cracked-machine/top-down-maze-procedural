#include <Components/AltarMultiBlock.hpp>
#include <Components/CryptExit.hpp>
#include <Components/CryptMultiBlock.hpp>
#include <Components/CryptObjectiveMultiBlock.hpp>
#include <Components/CryptObjectiveSegment.hpp>
#include <Components/CryptPassageBlock.hpp>
#include <Components/CryptPassageDoor.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>
#include <functional>
#include <unordered_map>

namespace ProceduralMaze::Sys
{

void CryptSystem::unlock_crypt_door()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

      // prevent spamming the activate key
      if ( m_door_cooldown_timer.getElapsedTime().asSeconds() < m_door_cooldown_time ) continue;
      m_door_cooldown_timer.restart();

      // Player can't intersect with a closed crypt door so expand their hitbox slightly to facilitate collision
      // detection
      auto increased_player_bounds = Cmp::RectBounds( pc_pos_cmp.position, pc_pos_cmp.size, 1.5f, Cmp::RectBounds::ScaleCardinality::VERTICAL );
      if ( not increased_player_bounds.findIntersection( door_pos_cmp ) ) continue;

      // Crypt door is already opened
      if ( cryptdoor_cmp.is_open() )
      {
        // Set the z-order value
        auto crypt_view = getReg().view<Cmp::CryptMultiBlock>();
        for ( auto [crypt_entity, crypt_cmp] : crypt_view.each() )
        {
          if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
          getReg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entity, crypt_cmp.position.y - 16.f );
        }
        continue;
      }
      else
      {
        // Set the z-order value
        auto crypt_view = getReg().view<Cmp::CryptMultiBlock>();
        for ( auto [crypt_entity, crypt_cmp] : crypt_view.each() )
        {
          if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
          getReg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entity, crypt_cmp.position.y + 16.f );
        }
      }

      // No keys to open the crypt door
      auto player_key_count = getReg().try_get<Cmp::PlayerKeysCount>( pc_entity );
      if ( player_key_count && player_key_count->get_count() == 0 )
      {
        m_sound_bank.get_effect( "crypt_locked" ).play();
        continue;
      }

      // unlock the crypt door
      SPDLOG_INFO( "Player unlocked a crypt door at ({}, {})", door_pos_cmp.position.x, door_pos_cmp.position.y );
      player_key_count->decrement_count( 1 );
      m_sound_bank.get_effect( "crypt_open" ).play();
      cryptdoor_cmp.set_is_open( true );

      // make doorway non-solid and lower z-order so player walks over it
      getReg().emplace_or_replace<Cmp::CryptSegment>( door_entity, Cmp::CryptSegment( false ) );

      // find the crypt multi-block this door belongs to and update the sprite
      auto crypt_view = getReg().view<Cmp::CryptMultiBlock, Cmp::SpriteAnimation>();
      for ( auto [crypt_entity, crypt_cmp, anim_cmp] : crypt_view.each() )
      {
        if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
        anim_cmp.m_sprite_type = "CRYPT.opened";

        SPDLOG_INFO( "Updated crypt multi-block sprite to open state at ({}, {})", crypt_cmp.position.x, crypt_cmp.position.y );
        break;
      }
    }
  }
}

void CryptSystem::check_entrance_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, crypt_door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), crypt_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds( crypt_door_pos_cmp.position, crypt_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_entrance_collision: Player entering crypt from graveyard at position ({}, {})", pc_pos_cmp.position.x,
                   pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_CRYPT );
    }
  }
}

void CryptSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptExit, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, crypt_door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), crypt_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds( crypt_door_pos_cmp.position, crypt_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_exit_collision: Player exiting crypt to graveyard at position ({}, {})", pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_CRYPT );
    }
  }
}

void CryptSystem::check_objective_activation( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerCadaverCount>();
  auto grave_view = getReg().view<Cmp::CryptObjectiveMultiBlock>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_cadaver_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );

    for ( auto [objective_entity, objective_cmp] : grave_view.each() )
    {
      // did we already get the cadaver from this objective?
      if ( objective_cmp.get_activation_count() >= objective_cmp.get_activation_threshold() ) continue;

      if ( player_hitbox.findIntersection( objective_cmp ) )
      {
        auto obst_entity = Factory::createLootDrop( getReg(), Cmp::SpriteAnimation{ 0, 0, true, "CADAVER_DROP", 0 },
                                                    sf::FloatRect{ objective_cmp.position, objective_cmp.size }, Factory::IncludePack<>{},
                                                    Factory::ExcludePack<Cmp::PlayableCharacter, Cmp::CryptObjectiveSegment>{} );
        if ( obst_entity != entt::null )
        {
          m_sound_bank.get_effect( "drop_loot" ).play();
          objective_cmp.increment_activation_count();
          SPDLOG_INFO( "Player activated crypt objective." );
        }
      }
    }
  }
}

void CryptSystem::spawn_exit( sf::Vector2u spawn_position )
{

  sf::FloatRect spawn_pos_px = sf::FloatRect( { static_cast<float>( spawn_position.x ) * Constants::kGridSquareSizePixels.x,
                                                static_cast<float>( spawn_position.y ) * Constants::kGridSquareSizePixels.y },
                                              Constants::kGridSquareSizePixelsF );

  // remove any wall
  for ( auto [entt, wall_cmp, pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
  {
    if ( spawn_pos_px.findIntersection( pos_cmp ) ) { getReg().destroy( entt ); }
  }

  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, spawn_pos_px.position, Constants::kGridSquareSizePixelsF );
  getReg().emplace_or_replace<Cmp::Exit>( entity, false ); // unlocked at start
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", 1 );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.position.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
  getReg().emplace_or_replace<Cmp::CryptExit>( entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

bool CryptSystem::place_passage_block( unsigned int passage_id, float x, float y, AllowDuplicatePassages duplicates_policy )
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
    if ( ( found_passage_block_pos_cmp.findIntersection( next_passage_block_cmp ) ) and
         ( duplicates_policy == CryptSystem::AllowDuplicatePassages::NO ) )
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
  SPDLOG_INFO( "Placed CryptPassageBlock at {},{} (id:{})", x, y, passage_id );

  return true;
};

bool CryptSystem::createDogLegPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, AllowDuplicatePassages duplicates_policy )
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

bool CryptSystem::createDrunkenWalkPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, std::set<entt::entity> exclude_entts,
                                            AllowDuplicatePassages duplicates_policy )
{
  // Generate unique walk ID
  m_current_passage_id++;
  SPDLOG_INFO( "createDrunkenWalkPassage (id:{}) from ({},{}) to ({},{})", m_current_passage_id, start.x, start.y, end_bounds.getCenter().x,
               end_bounds.getCenter().y );

  const float min_distance_between_walks = Constants::kGridSquareSizePixelsF.x * 2.0f;
  const int max_walk_length = 100; // Prevent infinite walks
  int walk_step_count = 0;

  // Check if this new walk would start too close to any existing walk from different walk IDs
  auto existing_blocks = getReg().view<Cmp::CryptPassageBlock>();

  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );
  sf::FloatRect walk_bounds( { 0.f, 0.f }, world_size );

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

  walk_bounds = sf::FloatRect( sf::Vector2f( min_x, min_y ), sf::Vector2f( max_x - min_x, max_y - min_y ) );

  SPDLOG_INFO( "walk_bounds = {},{} to {},{}", walk_bounds.position.x, walk_bounds.position.y, walk_bounds.position.x + walk_bounds.size.x,
               walk_bounds.position.y + walk_bounds.size.y );

  place_passage_block( m_current_passage_id, start.x, start.y, duplicates_policy );
  sf::FloatRect current_pos( { start.x, start.y }, Constants::kGridSquareSizePixelsF );

  auto direction_picker = Cmp::RandomInt( 0, 99 ); // 0-99 for percentage-based selection

  // Parameters to control oscillation
  const float bias_strength = 0.6f;     // 60% chance to move toward target
  const float momentum_strength = 0.1f; // 10% chance to continue in same direction
  sf::Vector2f last_move_direction( 0.f, 0.f );

  // Track recent positions to avoid immediate backtracking
  std::vector<sf::Vector2f> recent_positions;
  const size_t max_recent_positions = 5;

  // keep walking until we reach our target or hit limits
  while ( not current_pos.findIntersection( end_bounds ) && walk_step_count < max_walk_length )
  {
    sf::FloatRect candidate_pos( { -16.f, -16.f }, Constants::kGridSquareSizePixelsF );
    int attempts = 0;
    const int max_attempts = 50;
    bool is_candidate_rejected = false;
    bool found_valid_candidate = false;

    do
    {
      // Calculate direction towards target
      sf::Vector2f to_target = end_bounds.getCenter() - current_pos.getCenter();

      sf::Vector2f chosen_direction;
      int random_choice = direction_picker.gen();

      // Force initial steps to go in orthogonal direction from start
      if ( walk_step_count < 3 ) { chosen_direction = m_direction_dictionary[start.m_direction]; }
      else
      {
        if ( random_choice < static_cast<int>( bias_strength * 100 ) )
        {
          // Bias toward target: choose direction that reduces distance to target
          if ( std::abs( to_target.x ) > std::abs( to_target.y ) )
          {
            chosen_direction = ( to_target.x > 0 ) ? sf::Vector2f( 1.f, 0.f ) : sf::Vector2f( -1.f, 0.f );
          }
          else { chosen_direction = ( to_target.y > 0 ) ? sf::Vector2f( 0.f, 1.f ) : sf::Vector2f( 0.f, -1.f ); }
        }
        else if ( random_choice < static_cast<int>( ( bias_strength + momentum_strength ) * 100 ) &&
                  ( last_move_direction.x != 0.f || last_move_direction.y != 0.f ) )
        {
          // Continue in same direction (momentum)
          chosen_direction = last_move_direction;
        }
        else
        {
          // Random direction
          auto random_dir_idx = Cmp::RandomInt( 0, m_direction_choices.size() - 1 );
          chosen_direction = m_direction_choices[random_dir_idx.gen()];
        }
      }

      auto chosen_magnitude = chosen_direction.componentWiseMul( Constants::kGridSquareSizePixelsF );
      candidate_pos.position.x = current_pos.position.x + chosen_magnitude.x;
      candidate_pos.position.y = current_pos.position.y + chosen_magnitude.y;

      // Check if candidate is within bounds
      if ( !walk_bounds.contains( candidate_pos.position ) ||
           candidate_pos.position.x + candidate_pos.size.x > walk_bounds.position.x + walk_bounds.size.x ||
           candidate_pos.position.y + candidate_pos.size.y > walk_bounds.position.y + walk_bounds.size.y )
      {
        attempts++;
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

      if ( is_recent && attempts < max_attempts * 0.7f ) // Allow recent positions if we're running out of attempts
      {
        attempts++;
        continue;
      }

      // Check constraints
      is_candidate_rejected = false;

      // Check minimum distance from blocks belonging to different walks only
      if ( duplicates_policy == AllowDuplicatePassages::NO )
      {
        for ( auto [block_entt, block_cmp] : existing_blocks.each() )
        {
          if ( block_cmp.m_passage_id != m_current_passage_id )
          {
            float distance = Utils::Maths::getEuclideanDistance( candidate_pos.position, block_cmp );
            if ( distance < min_distance_between_walks )
            {
              is_candidate_rejected = true;
              SPDLOG_WARN( "Candidate rejected for passage id :{} - PassageBlock {},{} too close to candidate {},{}", m_current_passage_id,
                           block_cmp.x, block_cmp.y, candidate_pos.position.x, candidate_pos.position.y );
              break;
            }
          }
        }

        if ( !is_candidate_rejected )
        {
          for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
          {
            if ( exclude_entts.contains( open_room_entt ) ) continue;
            if ( walk_step_count > 3 )
            {
              Cmp::RectBounds scaled_candidate_pos( candidate_pos.position, candidate_pos.size, 2.f );
              if ( scaled_candidate_pos.findIntersection( open_room_cmp ) )
              {
                is_candidate_rejected = true;
                SPDLOG_WARN( "Candidate rejected for passage id :{} - Open room {},{} too close to candidate {},{}", m_current_passage_id,
                             open_room_cmp.position.x, open_room_cmp.position.y, candidate_pos.position.x, candidate_pos.position.y );
                break;
              }
            }
          }
        }
      }

      if ( !is_candidate_rejected )
      {
        for ( auto [wall_entt, wall_cmp, wall_pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
        {
          if ( candidate_pos.findIntersection( wall_pos_cmp ) )
          {
            is_candidate_rejected = true;
            SPDLOG_WARN( "Candidate rejected for passage id :{} - Wall {},{} too close to candidate {},{}", m_current_passage_id,
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

      attempts++;

    } while ( attempts < max_attempts );

    if ( !found_valid_candidate )
    {
      SPDLOG_WARN( "createDrunkenWalkPassage (id:{}): No valid candidate found after {} attempts, terminating walk", m_current_passage_id,
                   max_attempts );
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

  if ( walk_step_count >= max_walk_length )
  {
    SPDLOG_WARN( "createDrunkenWalkPassage (id:{}): Maximum walk length reached, terminating", m_current_passage_id );
    return false;
  }

  SPDLOG_INFO( "++++++++++++++++++++ Target Reached (id:{}) +++++++++++++++", m_current_passage_id );
  return true;
}

void CryptSystem::connectPassagesBetweenStartAndOpenRooms()
{

  auto [start_room_entt, start_room_cmp] = get_crypt_room_start();

  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );
  const auto start_room_right_pos_x = start_room_cmp.position.x + start_room_cmp.size.x;

  // divide the gamrarea into 3 quadrants - again there are only three because startroom is southern most position in the game area
  auto west_quad = sf::FloatRect( { 0.f, 0.f }, { start_room_cmp.position.x, world_size.y } );
  SPDLOG_INFO( "west_quadrant: {},{} : {},{}", west_quad.position.x, west_quad.position.y, west_quad.size.x, west_quad.size.y );

  auto east_quad = sf::FloatRect( { start_room_right_pos_x, 0.f }, { world_size.x - ( start_room_right_pos_x ), world_size.y } );
  SPDLOG_INFO( "east_quadrant: {},{} : {},{}", east_quad.position.x, east_quad.position.y, east_quad.size.x, east_quad.size.y );

  auto north_quad = sf::FloatRect( { 0.f, 0.f }, { world_size.x, start_room_cmp.position.y } );
  SPDLOG_INFO( "north_quadrant: {},{} : {},{}", north_quad.position.x, north_quad.position.y, north_quad.size.x, north_quad.size.y );

  find_passage_target( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::WEST], west_quad, { start_room_entt } );
  find_passage_target( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::EAST], east_quad, { start_room_entt } );
  find_passage_target( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH], north_quad, { start_room_entt } );
  tidyPassageBlocks();
}

void CryptSystem::connectPassagesBetweenOccupiedAndOpenRooms()
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

void CryptSystem::connectPassagesBetweenAllOpenRooms()
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

void CryptSystem::connectPassageBetweenOccupiedAndEndRoom()
{
  auto [crypt_room_end_entt, crypt_end_room_cmp] = get_crypt_room_end();

  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( not Utils::get_player_position( getReg() ).findIntersection( open_room_cmp ) ) continue;
    auto &occupied_room_cmp = open_room_cmp;

    // no need to search for suitable target, we already have it
    auto current_passage_door = occupied_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH];
    createDrunkenWalkPassage( current_passage_door, crypt_end_room_cmp, { open_room_entt, crypt_room_end_entt } );
  }

  tidyPassageBlocks();
}

void CryptSystem::find_passage_target( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant,
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
      if ( passage_limit == CryptSystem::OnePassagePerTargetRoom::YES )
      {
        nearest_other_room_cmp.set_all_doors_used( true ); // prevent more passages
      }
    }
  }
}

void CryptSystem::closeOpenRooms()
{

  std::vector<std::pair<entt::entity, Cmp::CryptRoomOpen>> rooms_to_close;
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( open_room_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;
    rooms_to_close.push_back( { open_room_entt, open_room_cmp } );
  }

  // close the entities safely outside of view
  for ( auto [room_entt, room_cmp] : rooms_to_close )
  {
    getReg().emplace<Cmp::CryptRoomClosed>( room_entt, room_cmp.position, room_cmp.size );
    getReg().remove<Cmp::CryptRoomOpen>( room_entt );
  }
}

void CryptSystem::fillClosedRooms()
{
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
    {
      // skip position outside closed room area or if position already has existing obstacle
      if ( not closed_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      Factory::createObstacle( getReg(), pos_entt, pos_cmp, obst_type, 5, ( zorder * 2.f ) );
    }
  }
}

void CryptSystem::openSelectedRooms( std::set<entt::entity> selected_rooms )
{
  std::vector<std::pair<entt::entity, Cmp::CryptRoomClosed>> rooms_to_open;
  auto closed_room_view = getReg().view<Cmp::CryptRoomClosed>();
  for ( auto [closed_room_entt, closed_room_cmp] : closed_room_view.each() )
  {
    if ( selected_rooms.find( closed_room_entt ) == selected_rooms.end() ) continue;

    // save the CryptRoomClosed entities we want to change to CryptRoomOpen
    rooms_to_open.push_back( { closed_room_entt, closed_room_cmp } );
  }

  // open the room safely outside of view loop
  for ( auto [room_entt, room_cmp] : rooms_to_open )
  {
    getReg().emplace<Cmp::CryptRoomOpen>( room_entt, room_cmp.position, room_cmp.size );
    getReg().remove<Cmp::CryptRoomClosed>( room_entt );
  }
}

void CryptSystem::emptyOpenRooms()
{
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      // skip position outside open room area or if position doesn't have existing obstacle
      if ( not open_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( not getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      Factory::destroyObstacle( getReg(), pos_entt );
    }
  }
}

void CryptSystem::removeAllPassageBlocks()
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

void CryptSystem::emptyOpenPassages()
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

void CryptSystem::fillAllPassages()
{
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    auto pblock_view = getReg().view<Cmp::CryptPassageBlock>();
    for ( auto [pblock_entt, pblock_cmp] : pblock_view.each() )
    {
      auto pblock_cmp_rect = sf::FloatRect( pblock_cmp, Constants::kGridSquareSizePixelsF );
      // skip any positions that are not pblocks or already have obstacles
      if ( not pblock_cmp_rect.findIntersection( pos_cmp ) ) continue;
      if ( getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      Factory::createObstacle( getReg(), pos_entt, pos_cmp, obst_type, 5, ( zorder * 2.f ) );
    }
  }
}

void CryptSystem::tidyPassageBlocks( bool include_closed_rooms )
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

void CryptSystem::createRoomBorders()
{
  SPDLOG_INFO( "createRoomBorders" );
  auto add_border = [&]<typename Component>( entt::entity pos_entt, Cmp::Position &pos_cmp, Component &room_cmp, Sprites::SpriteMetaType sprite_type,
                                             size_t sprite_index )
  {
    bool is_border = false;

    float room_left = room_cmp.position.x;
    float room_right = room_cmp.position.x + room_cmp.size.x;
    float room_top = room_cmp.position.y;
    float room_bottom = room_cmp.position.y + room_cmp.size.y;

    // clang-format off
    // left border
    if ( ( pos_cmp.position.x == room_left - Constants::kGridSquareSizePixelsF.x ) and
         ( pos_cmp.position.y >= room_top - Constants::kGridSquareSizePixelsF.y
           and pos_cmp.position.y <= room_bottom ) )
    {
      is_border = true;
    }
    // right border
    else if ( ( pos_cmp.position.x == room_right ) and
              ( pos_cmp.position.y >= room_top - Constants::kGridSquareSizePixelsF.y and 
                pos_cmp.position.y <= room_bottom ) )
    {
      is_border = true;
    }
    // top border
    else if ( ( pos_cmp.position.y == room_top - Constants::kGridSquareSizePixelsF.y ) and
              ( pos_cmp.position.x >= room_left - Constants::kGridSquareSizePixelsF.x and 
                pos_cmp.position.x <= room_right ) )
    {
      is_border = true;
    }
    // bottom border
    else if ( ( pos_cmp.position.y == room_bottom ) and
              ( pos_cmp.position.x >= room_left - Constants::kGridSquareSizePixelsF.x and 
                pos_cmp.position.x <= room_right ) )
    {
      is_border = true;
    }
    // clang-format on

    if ( is_border )
    {
      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { sprite_type } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( sprite_type ).y;
      Factory::createObstacle( getReg(), pos_entt, pos_cmp, obst_type, sprite_index, ( zorder * 2.f ) );
      SPDLOG_INFO( "Created Border" );
    }
    else { SPDLOG_INFO( "Failed to create border" ); }
  };

  for ( auto [pos_entt, pos_cmp] : getReg().view<Cmp::Position>().each() )
  {
    // replace closed room borders with regular sprites
    for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
    {
      add_border( pos_entt, pos_cmp, closed_room_cmp, "CRYPT.interior_sb", 5 );
    }
    // Always add end room border
    for ( auto [end_room_entt, end_room_cmp] : getReg().view<Cmp::CryptRoomEnd>().each() )
    {
      add_border( pos_entt, pos_cmp, end_room_cmp, "WALL", 0 );
    }
    // replace open room borders with actual border sprite
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      add_border( pos_entt, pos_cmp, open_room_cmp, "WALL", 0 );
    }
  }
}

void CryptSystem::on_room_event( Events::CryptRoomEvent &event )
{
  m_current_passage_id = 0;
  if ( event.type == Events::CryptRoomEvent::Type::SHUFFLE_PASSAGES )
  {
    auto selected_rooms = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomClosed>(
        getReg(), 4, {}, Utils::Rnd::ExcludePack<Cmp::CryptRoomStart, Cmp::CryptRoomEnd>{}, 0 );

    // reset rooms/passages
    closeOpenRooms();
    fillClosedRooms();
    fillAllPassages();
    removeAllPassageBlocks();

    // open new rooms/passages
    openSelectedRooms( selected_rooms );
    emptyOpenRooms();
    createRoomBorders();
    SPDLOG_INFO( "~~~~~~~~~~~ STARTING PASSAGE GEN ~~~~~~~~~~~~~~~" );
    // try to open passages for the occupied room: only do start room if player is currently there
    auto [start_room_entt, start_room_cmp] = get_crypt_room_start();
    if ( Utils::get_player_position( getReg() ).findIntersection( start_room_cmp ) ) { connectPassagesBetweenStartAndOpenRooms(); }
    else { connectPassagesBetweenOccupiedAndOpenRooms(); }
    emptyOpenPassages();
  }
  else if ( event.type == Events::CryptRoomEvent::Type::FINAL_PASSAGE )
  {
    // reset rooms/passages
    closeOpenRooms();
    fillClosedRooms();
    fillAllPassages();
    removeAllPassageBlocks();
    createRoomBorders();

    // open new rooms/passages
    SPDLOG_INFO( "~~~~~~~~~~~ OPENING FINAL PASSAGE ~~~~~~~~~~~~~~~" );
    connectPassageBetweenOccupiedAndEndRoom();
    emptyOpenPassages();
  }
  else if ( event.type == Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES )
  {
    // we need to open all rooms, its easier to reuse existing code: close all rooms and then use 'openSelectedRooms()' using a set of ALL rooms
    //! @todo This could be opotimized to be more efficient.
    closeOpenRooms();
    std::set<entt::entity> all_rooms;
    for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
    {
      all_rooms.insert( closed_room_entt );
    }
    openSelectedRooms( all_rooms );
    emptyOpenRooms();
    createRoomBorders();
    connectPassagesBetweenStartAndOpenRooms(); // make sure player can reach exit
    connectPassagesBetweenAllOpenRooms();
    emptyOpenPassages();
  }
}

} // namespace ProceduralMaze::Sys