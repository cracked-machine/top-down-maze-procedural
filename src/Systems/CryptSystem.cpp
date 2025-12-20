#include <Components/AltarMultiBlock.hpp>
#include <Components/CryptExit.hpp>
#include <Components/CryptMultiBlock.hpp>
#include <Components/CryptObjectiveMultiBlock.hpp>
#include <Components/CryptObjectiveSegment.hpp>
#include <Components/CryptPassageBlock.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>
#include <functional>

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

      // make sure player returns just below the crypt entrance to prevent infinite re-entry
      // m_player_last_known_graveyard_pos = sf::Vector2f( pc_pos_cmp.position.x,
      //                                                   pc_pos_cmp.position.y + Constants::kGridSquareSizePixels.y );

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

      // // set player position as they return to the graveyard
      // pc_pos_cmp.position = sf::Vector2f(
      //     static_cast<float>( m_player_last_known_graveyard_pos.x ) * Constants::kGridSquareSizePixels.x,
      //     static_cast<float>( m_player_last_known_graveyard_pos.y ) * Constants::kGridSquareSizePixels.y );
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

void CryptSystem::closeAllRooms()
{

  std::vector<std::pair<entt::entity, Cmp::CryptRoomOpen>> rooms_to_close;
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( open_room_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;

    // close all opened room by adding obstacle within its boundary
    auto position_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::PlayableCharacter, Cmp::ReservedPosition> );
    for ( auto [entity, pos_cmp] : position_view.each() )
    {
      if ( not pos_cmp.findIntersection( open_room_cmp ) ) continue;

      // clang-format off
      auto [obst_type, rand_obst_tex_idx] = 
        m_sprite_factory.get_random_type_and_texture_index( { 
          "CRYPT.interior_sb"
        } );
      // clang-format on

      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      Factory::createObstacle( getReg(), entity, pos_cmp, obst_type, 5, ( zorder * 2.f ) );
    }

    // save the CryptRoomOpen entities we want change to CryptRoomClosed
    rooms_to_close.push_back( { open_room_entt, open_room_cmp } );
  }

  // close the entities safely outside of view
  for ( auto [room_entt, room_cmp] : rooms_to_close )
  {
    getReg().emplace<Cmp::CryptRoomClosed>( room_entt, room_cmp.position, room_cmp.size );
    getReg().remove<Cmp::CryptRoomOpen>( room_entt );
  }
}

void CryptSystem::openSelectedRooms( std::set<entt::entity> selected_rooms )
{
  std::vector<std::pair<entt::entity, Cmp::CryptRoomClosed>> rooms_to_open;
  auto closed_room_view = getReg().view<Cmp::CryptRoomClosed>();
  for ( auto [closed_room_entt, closed_room_cmp] : closed_room_view.each() )
  {
    if ( selected_rooms.find( closed_room_entt ) == selected_rooms.end() ) continue;

    // open the selected room by removing any obstacles inside its boundary
    auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [obst_entity, obst_cmp, obst_pos_cmp] : obstacle_view.each() )
    {
      if ( not closed_room_cmp.findIntersection( obst_pos_cmp ) ) continue;
      Factory::destroyObstacle( getReg(), obst_entity );
    }

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

bool CryptSystem::place_passage_block( float x, float y, std::vector<entt::entity> &new_block_list )
{

  auto block_list_unwind = [&]()
  {
    for ( auto &passage_block_entity : new_block_list )
    {
      getReg().destroy( passage_block_entity );
    }
    new_block_list.clear();
  };

  Cmp::Position next_passage_block_cmp( { x, y }, Constants::kGridSquareSizePixelsF );

  // Check if a block already exists at this position
  auto block_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [passage_block_entt, passage_block_cmp] : block_view.each() )
  {
    Cmp::Position found_passage_block_pos_cmp( passage_block_cmp, Constants::kGridSquareSizePixelsF );
    if ( found_passage_block_pos_cmp.findIntersection( next_passage_block_cmp ) )
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
  getReg().emplace_or_replace<Cmp::CryptPassageBlock>( new_entt, next_passage_block_cmp.position );
  new_block_list.push_back( new_entt );
  SPDLOG_INFO( "Placed CryptPassageBlock at {},{}", x, y );

  return true;
};

bool CryptSystem::createDogLegPassage( sf::Vector2f start, sf::Vector2f end )
{
  SPDLOG_INFO( "Entered createDogLegPassage from ({},{}) to ({},{})", start.x, start.y, end.x, end.y );

  std::vector<entt::entity> new_block_list;

  float dx = end.x - start.x;
  float dy = end.y - start.y;
  const auto kSquareSizePx = Constants::kGridSquareSizePixelsF;

  if ( std::abs( dx ) > std::abs( dy ) )
  {
    SPDLOG_INFO( "start with horizontal leg" );
    // First leg: horizontal
    if ( dx > 0 )
    {
      for ( float x = start.x; x <= end.x; x += kSquareSizePx.x )
      {
        if ( not place_passage_block( x, start.y, new_block_list ) ) { return false; }
      }
    }
    else
    {
      for ( float x = start.x; x >= end.x; x -= kSquareSizePx.x )
      {
        if ( not place_passage_block( x, start.y, new_block_list ) ) { return false; }
      }
    }
    SPDLOG_INFO( "switch to vertical leg" );
    // Second leg: vertical from end.x to end.y
    if ( dy > 0 )
    {
      for ( float y = start.y + kSquareSizePx.y; y <= end.y; y += kSquareSizePx.y )
      {
        if ( not place_passage_block( end.x, y, new_block_list ) ) { return false; }
      }
    }
    else
    {
      for ( float y = start.y - kSquareSizePx.y; y >= end.y; y -= kSquareSizePx.y )
      {
        if ( not place_passage_block( end.x, y, new_block_list ) ) { return false; }
      }
    }
  }
  else
  {
    SPDLOG_INFO( "start with vertical leg" );
    // First leg: vertical
    if ( dy > 0 )
    {
      for ( float y = start.y; y <= end.y; y += kSquareSizePx.y )
      {
        if ( not place_passage_block( start.x, y, new_block_list ) ) { return false; }
      }
    }
    else
    {
      for ( float y = start.y; y >= end.y; y -= kSquareSizePx.y )
      {
        if ( not place_passage_block( start.x, y, new_block_list ) ) { return false; }
      }
    }
    SPDLOG_INFO( "switch to horizontal leg" );
    // Second leg: horizontal from start.x to end.x
    if ( dx > 0 )
    {
      for ( float x = start.x + kSquareSizePx.x; x <= end.x; x += kSquareSizePx.x )
      {
        if ( not place_passage_block( x, end.y, new_block_list ) ) { return false; }
      }
    }
    else
    {
      for ( float x = start.x - kSquareSizePx.x; x >= end.x; x -= kSquareSizePx.x )
      {
        if ( not place_passage_block( x, end.y, new_block_list ) ) { return false; }
      }
    }
  }

  return true;
}

void CryptSystem::openRandomPassages()
{

  auto start_room_cmp = get_crypt_room_start();
  if ( not Utils::get_player_position( getReg() ).findIntersection( start_room_cmp ) ) return;

  // get view of all "other" rooms i.e. open rooms - this is a small number 3-4
  const auto world_size = Scene::CryptScene::kMapGridSizeF.componentWiseMul( Constants::kGridSquareSizePixelsF );

  // divide the gamrarea into 3 quadrants - again there are only three because startroom is southern most position in the game area
  auto west_quadrant = sf::FloatRect( { 0.f, 0.f }, { start_room_cmp.position.x, world_size.y } );
  SPDLOG_INFO( "west_quadrant: {},{} : {},{}", west_quadrant.position.x, west_quadrant.position.y, west_quadrant.size.x, west_quadrant.size.y );
  auto east_quadrant = sf::FloatRect( { start_room_cmp.position.x + start_room_cmp.size.x, 0.f },
                                      { world_size.x - ( start_room_cmp.position.x + start_room_cmp.size.x ), world_size.y } );
  SPDLOG_INFO( "east_quadrant: {},{} : {},{}", east_quadrant.position.x, east_quadrant.position.y, east_quadrant.size.x, east_quadrant.size.y );
  auto north_quadrant = sf::FloatRect( { 0.f, 0.f }, { world_size.x, start_room_cmp.position.y } );
  SPDLOG_INFO( "north_quadrant: {},{} : {},{}", north_quadrant.position.x, north_quadrant.position.y, north_quadrant.size.x, north_quadrant.size.y );

  add_cardinal_passage( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::WEST], west_quadrant );
  add_cardinal_passage( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::EAST], east_quadrant );
  add_cardinal_passage( start_room_cmp.m_midpoints[Cmp::CryptPassageDirection::NORTH], north_quadrant );
}

void CryptSystem::add_cardinal_passage( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant )
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
    if ( not other_room_cmp.findIntersection( search_quadrant ) ) continue;
    if ( other_room_cmp.are_all_doors_used() ) continue;
    dist_pqueue.push( { Utils::Maths::getEuclideanDistance( start_passage_door, other_room_cmp.getCenter() ), other_room_entt } );
  }

  SPDLOG_INFO( "north_dist_pqueue - size:{}", dist_pqueue.size() );
  if ( not dist_pqueue.empty() )
  {
    auto nearest_north_other_room_entt = dist_pqueue.top().second;
    auto &nearest_north_other_room_cmp = getReg().get<Cmp::CryptRoomOpen>( nearest_north_other_room_entt );
    if ( createDogLegPassage( start_passage_door, nearest_north_other_room_cmp.getCenter() ) )
    {
      start_passage_door.is_used = true;
      nearest_north_other_room_cmp.set_all_doors_used( true ); // prevent more passages
    }
  }
}

void CryptSystem::closeAllPassages()
{
  std::vector<entt::entity> shit_list;

  auto crypt_passage_block_view = getReg().view<Cmp::CryptPassageBlock>();
  for ( auto [entt, block_cmp] : crypt_passage_block_view.each() )
  {
    //! @todo ReAdd obstacles

    shit_list.push_back( entt );
  }

  // Remove Cmp::CryptPassageBlocks safely
  for ( auto entt : shit_list )
  {
    getReg().remove<Cmp::CryptPassageBlock>( entt );
    getReg().destroy( entt );
  }
  auto crypt_passage_block_view_remaining = getReg().view<Cmp::CryptPassageBlock>();
  SPDLOG_INFO( "Remaining Cmp::CryptPassageBlock entities: {}", crypt_passage_block_view_remaining.size() );
}

void CryptSystem::on_room_event( Events::CryptRoomEvent &event )
{
  if ( event.type == Events::CryptRoomEvent::Type::SWAP )
  {
    auto selected_rooms = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomClosed>(
        getReg(), 4, {}, Utils::Rnd::ExcludePack<Cmp::CryptRoomStart, Cmp::CryptRoomEnd>{}, 0 );
    closeAllRooms();
    openSelectedRooms( selected_rooms );
    SPDLOG_INFO( "~~~~~~~~~~~ STARTING PASSAGE GEN ~~~~~~~~~~~~~~~" );
    closeAllPassages();
    openRandomPassages();
  }
}

} // namespace ProceduralMaze::Sys