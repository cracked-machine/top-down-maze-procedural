#include <Audio/SoundBank.hpp>
#include <Components/AltarMultiBlock.hpp>
#include <Components/CryptEntrance.hpp>
#include <Components/CryptExit.hpp>
#include <Components/CryptLever.hpp>
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
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/CryptNpcSpawnCount.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/Events/PassageEvent.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sys
{

void CryptSystem::update()
{
  if ( not m_maze_unlocked and Scene::CryptScene::is_maze_timer_expired() )
  {
    //
    shuffle_rooms_passages();
  }
}

void CryptSystem::on_player_action( Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) unlock_crypt_door();
  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_objective_activation( event.action );
  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_lever_activation( event.action );
}

void CryptSystem::on_room_event( Events::CryptRoomEvent &event )
{

  if ( event.type == Events::CryptRoomEvent::Type::SHUFFLE_PASSAGES ) { shuffle_rooms_passages(); }
  else if ( event.type == Events::CryptRoomEvent::Type::FINAL_PASSAGE ) { unlock_objective_passage(); }
  else if ( event.type == Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) { unlock_exit_passage(); }
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
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "CRYPT.interior_sb", 1 );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.position.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
  getReg().emplace_or_replace<Cmp::CryptExit>( entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

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
        sf::FloatRect expanded_search( sf::Vector2f{ objective_cmp.position.x, objective_cmp.position.y + objective_cmp.size.y },
                                       sf::Vector2f{ objective_cmp.size.x, Constants::kGridSquareSizePixelsF.y * 2.f } );
        auto obst_entity = Factory::createLootDrop( getReg(), Cmp::SpriteAnimation{ 0, 0, true, "CADAVER_DROP", 0 }, expanded_search,
                                                    Factory::IncludePack<>{},
                                                    Factory::ExcludePack<Cmp::PlayableCharacter, Cmp::CryptObjectiveSegment>{} );
        if ( obst_entity != entt::null )
        {
          m_sound_bank.get_effect( "drop_loot" ).play();
          objective_cmp.increment_activation_count();
          SPDLOG_INFO( "Player activated crypt objective." );

          const auto &ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_objective_opened" );
          getReg().emplace_or_replace<Cmp::SpriteAnimation>( objective_entity, 0, 0, true, ms.get_sprite_type(), 0 );
        }
      }
    }
  }
}

void CryptSystem::check_lever_activation( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;
  if ( m_maze_unlocked ) return;

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto lever_view = getReg().view<Cmp::CryptLever, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
    for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : lever_view.each() )
    {
      // prevent player from spamming lever twice
      if ( lever_cmp.isEnabled() ) continue;
      if ( not player_hitbox.findIntersection( lever_pos_cmp ) ) continue;

      lever_cmp.setEnabled( true );
      m_enabled_levers++;

      // update the sprite
      Sprites::SpriteMetaType lever_sprite_type = "LEVER";
      unsigned int enabled_lever_sprite_idx = 1;
      getReg().emplace_or_replace<Cmp::SpriteAnimation>( lever_entt, 0, 0, true, lever_sprite_type, enabled_lever_sprite_idx );

      m_sound_bank.get_effect( "crypt_lever_open" ).play();
      SPDLOG_INFO( "Lever enabled at {},{} - Count:{}", lever_pos_cmp.position.x, lever_pos_cmp.position.y, m_enabled_levers );

      // check if we have activated enough levers to access the maze objective
      if ( m_enabled_levers > 2 and not m_maze_unlocked )
      {
        m_maze_unlocked = true;

        unlock_objective_passage();
        Scene::CryptScene::get_maze_timer().reset();
      }
      else { shuffle_rooms_passages(); }
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
    }
  };

  for ( auto [pos_entt, pos_cmp] : getReg().view<Cmp::Position>().each() )
  {
    // don't add obstacles to footstep entities
    if ( getReg().any_of<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Direction>( pos_entt ) ) continue;
    // don't replace wall/exit entities
    if ( getReg().any_of<Cmp::Wall, Cmp::CryptExit>( pos_entt ) ) continue;

    // replace closed room borders with regular sprites
    for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
    {
      add_border( pos_entt, pos_cmp, closed_room_cmp, "CRYPT.interior_sb", 2 );
    }
    // Always add end room border
    for ( auto [end_room_entt, end_room_cmp] : getReg().view<Cmp::CryptRoomEnd>().each() )
    {
      add_border( pos_entt, pos_cmp, end_room_cmp, "CRYPT.interior_sb", 3 );
    }
    // Always add end room border
    for ( auto [start_room_entt, start_room_cmp] : getReg().view<Cmp::CryptRoomStart>().each() )
    {
      add_border( pos_entt, pos_cmp, start_room_cmp, "CRYPT.interior_sb", 3 );
    }
    // replace open room borders with actual border sprite
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      add_border( pos_entt, pos_cmp, open_room_cmp, "CRYPT.interior_sb", 3 );
    }
  }
}

/// PRIVATE FUNCTIONS

void CryptSystem::shuffle_rooms_passages()
{
  auto selected_rooms = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomClosed>(
      getReg(), 4, {}, Utils::Rnd::ExcludePack<Cmp::CryptRoomStart, Cmp::CryptRoomEnd>{}, 0 );

  // reset rooms/passages
  closeOpenRooms();
  removeLeverOpenRooms();
  fillClosedRooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::REMOVE_PASSAGES ) );

  // open new rooms/passages
  openSelectedRooms( selected_rooms );
  emptyOpenRooms();
  createRoomBorders();
  addLeverOpenRooms();

  // try to open passages for the occupied room: only do start room if player is currently there
  SPDLOG_INFO( "~~~~~~~~~~~ STARTING PASSAGE GEN ~~~~~~~~~~~~~~~" );
  auto [start_room_entt, start_room_cmp] = get_crypt_room_start();
  if ( Utils::get_player_position( getReg() ).findIntersection( start_room_cmp ) )
  {
    get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, start_room_entt ) );
  }
  else { get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_OPENROOMS ) ); }

  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );
  m_sound_bank.get_effect( "crypt_room_shuffle" ).play();
  Scene::CryptScene::get_maze_timer().restart();
}

void CryptSystem::unlock_objective_passage()
{
  // reset rooms/passages
  closeOpenRooms();
  removeAllLevers();
  fillClosedRooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::REMOVE_PASSAGES ) );
  createRoomBorders();

  // open new rooms/passages
  SPDLOG_INFO( "~~~~~~~~~~~ OPENING FINAL PASSAGE ~~~~~~~~~~~~~~~" );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_ENDROOM, get_crypt_room_end().first ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );
  m_sound_bank.get_effect( "crypt_room_shuffle" ).play();
}

void CryptSystem::unlock_exit_passage()
{
  // if we unlocked the maze by picking up the cadaver, then cancel the timer
  Scene::CryptScene::get_maze_timer().reset();
  m_maze_unlocked = true;

  closeOpenRooms();
  removeLeverOpenRooms();
  openAllRooms();
  emptyOpenRooms();
  createRoomBorders();

  // make sure player can reach exit
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, get_crypt_room_start().first ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_ALL_OPENROOMS ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );

  spawnNpcInOpenRooms();
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
    // don't add obstacles to footstep entities
    if ( getReg().any_of<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Direction>( pos_entt ) ) continue;

    for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
    {
      // skip position outside closed room area or if position already has existing obstacle
      if ( not closed_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( getReg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      Factory::createObstacle( getReg(), pos_entt, pos_cmp, obst_type, 2, ( zorder * 2.f ) );
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

void CryptSystem::openAllRooms()
{
  std::set<entt::entity> all_rooms;
  for ( auto [closed_room_entt, closed_room_cmp] : getReg().view<Cmp::CryptRoomClosed>().each() )
  {
    all_rooms.insert( closed_room_entt );
  }
  openSelectedRooms( all_rooms );
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

void CryptSystem::addLeverOpenRooms()
{
  // find all candidate positions in open rooms
  std::vector<entt::entity> internal_room_entts;
  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [pos_entt, pos_cmp] : obstacle_view.each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( not open_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( open_room_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;
      if ( pos_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;

      // Find passage blocks for this room to determine opposite placement
      sf::Vector2f passage_center{ 0, 0 };
      int passage_count = 0;

      auto passage_view = getReg().view<Cmp::CryptPassageBlock, Cmp::Position>();
      for ( auto [passage_entt, passage_cmp, passage_pos_cmp] : passage_view.each() )
      {
        // Check if passage block is on the border of this open room
        float room_left = open_room_cmp.position.x;
        float room_right = open_room_cmp.position.x + open_room_cmp.size.x;
        float room_top = open_room_cmp.position.y;
        float room_bottom = open_room_cmp.position.y + open_room_cmp.size.y;

        // Check if passage is adjacent to room border
        bool is_adjacent = false;
        if ( ( passage_pos_cmp.position.x >= room_left - Constants::kGridSquareSizePixelsF.x && passage_pos_cmp.position.x <= room_right ) &&
             ( passage_pos_cmp.position.y >= room_top - Constants::kGridSquareSizePixelsF.y && passage_pos_cmp.position.y <= room_bottom ) )
        {
          is_adjacent = true;
        }

        if ( is_adjacent )
        {
          passage_center.x += passage_pos_cmp.position.x + passage_pos_cmp.size.x / 2.0f;
          passage_center.y += passage_pos_cmp.position.y + passage_pos_cmp.size.y / 2.0f;
          passage_count++;
        }
      }

      // If we found passages, calculate their average center
      if ( passage_count > 0 )
      {
        passage_center.x /= passage_count;
        passage_center.y /= passage_count;

        // Get room center
        sf::Vector2f room_center = open_room_cmp.getCenter();

        // Calculate which side the passages are on relative to room center
        sf::Vector2f passage_direction = passage_center - room_center;

        // Determine opposite position preference
        sf::Vector2f opposite_target = room_center - passage_direction;

        // Only add positions that are closer to the opposite side
        sf::Vector2f pos_center = pos_cmp.position + pos_cmp.size / 2.0f;
        float dist_to_opposite = Utils::Maths::getEuclideanDistance( pos_center, opposite_target );
        float dist_to_passage = Utils::Maths::getEuclideanDistance( pos_center, passage_center );

        // Only include positions that are closer to opposite side than to passages
        if ( dist_to_opposite < dist_to_passage ) { internal_room_entts.push_back( pos_entt ); }
      }
      else
      {
        // No passages found for this room, any position is valid
        internal_room_entts.push_back( pos_entt );
      }
    }
  }

  if ( internal_room_entts.empty() )
  {
    SPDLOG_WARN( "No valid positions found for lever placement" );
    return;
  }

  Sprites::SpriteMetaType lever_sprite_type = "LEVER";
  unsigned int disabled_lever_sprite_idx = 0;
  float zorder = m_sprite_factory.get_sprite_size_by_type( lever_sprite_type ).y;

  // add one lever to one room picked from the pool of candidates room positions
  Cmp::RandomInt room_position_picker( 0, internal_room_entts.size() - 1 );
  auto selected_entt = internal_room_entts[room_position_picker.gen()];
  auto room_pos = getReg().get<Cmp::Position>( selected_entt );
  Factory::CreateCryptLever( getReg(), room_pos.position, lever_sprite_type, disabled_lever_sprite_idx, zorder );
  SPDLOG_INFO( "Added lever to position: {},{}", room_pos.position.x, room_pos.position.y );
}

void CryptSystem::removeLeverOpenRooms()
{
  for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : getReg().view<Cmp::CryptLever, Cmp::Position>().each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( not open_room_cmp.findIntersection( lever_pos_cmp ) ) continue;
      if ( open_room_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;

      Factory::DestroyCryptLever( getReg(), lever_entt );
    }
  }
}

void CryptSystem::removeAllLevers()
{
  for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : getReg().view<Cmp::CryptLever, Cmp::Position>().each() )
  {
    Factory::DestroyCryptLever( getReg(), lever_entt );
  }
}

void CryptSystem::spawnNpcInOpenRooms()
{
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  std::vector<Cmp::CryptRoomOpen> open_room_list;

  // Collect all open rooms
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    open_room_list.push_back( open_room_cmp );
  }

  // Early return if not enough rooms
  if ( open_room_list.size() < 2 )
  {
    SPDLOG_WARN( "Not enough open rooms to spawn 2 NPCs. Available rooms: {}", open_room_list.size() );
    return;
  }

  // Spawn user specified number of NPCs or the max number of rooms: whichever is smallest.
  const int npcs_to_spawn = std::min( Sys::PersistSystem::get_persist_cmp<Cmp::Persist::CryptNpcSpawnCount>( getReg() ).get_value(),
                                      static_cast<unsigned short>( open_room_list.size() ) );

  for ( int r = 0; r < npcs_to_spawn; r++ )
  {
    // Select a random room from remaining rooms
    Cmp::RandomInt room_picker( 0, open_room_list.size() - 1 );
    std::size_t selected_idx = room_picker.gen();

    // Extract the selected room
    auto extracted_open_room_cmp = std::move( open_room_list[selected_idx] );

    if ( selected_idx < open_room_list.size() - 1 )
    {
      // Remove selected room using swap and pop to maintain efficiency
      open_room_list[selected_idx] = std::move( open_room_list.back() );
    }
    open_room_list.pop_back();

    // Spawn NPC in the selected room
    auto spawn_position = Utils::snap_to_grid( extracted_open_room_cmp.getCenter() );
    auto position_entity = getReg().create();
    Cmp::Position position_cmp = getReg().emplace<Cmp::Position>( position_entity, spawn_position, Constants::kGridSquareSizePixelsF );
    [[maybe_unused]] Cmp::ZOrderValue zorder_cmp = getReg().emplace<Cmp::ZOrderValue>( position_entity, position_cmp.position.y );
    Factory::createNPC( getReg(), position_entity, "NPCPRIEST" );

    SPDLOG_INFO( "Spawned NPC {} at position ({}, {})", r + 1, spawn_position.x, spawn_position.y );
  }
}

} // namespace ProceduralMaze::Sys