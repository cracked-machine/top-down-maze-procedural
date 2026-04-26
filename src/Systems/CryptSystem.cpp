#include <Constants.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Player/PlayerNoPath.hpp>
#include <VoidPosition.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptPassageDoor.hpp>
#include <Components/Crypt/CryptPassageSpikeTrap.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomLavaPit.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomLavaPitCellEffect.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/CryptNpcSpawnCount.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/Audio/Sound.hpp>
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
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sys
{

void CryptSystem::setup()
{

  if ( Utils::Player::get_mortality( m_reg ).state != Cmp::PlayerMortality::State::DEAD )
  {
    createRoomBorders();

    // make sure player has been situated in start room first
    shuffle_rooms_passages();
    reset_maze();
  }
}

void CryptSystem::update()
{

  check_exit_collision();

  if ( not m_maze_unlocked and Scene::CryptScene::is_maze_timer_expired() )
  {
    //
    shuffle_rooms_passages();
  }

  // check collisions with lava pit
  if ( not Utils::getSystemCmp( reg() ).collisions_disabled )
  {
    check_lava_pit_collision();
    check_spike_trap_collision();
  }
  check_lever_activation(); // useful for debugging
  check_lava_pit_activation_by_proximity();
  do_lava_pit_animation();
  check_spike_trap_activation_by_proximity();
}

void CryptSystem::on_player_action( Events::PlayerActionEvent &event )
{
  if ( Utils::Player::get_mortality( reg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_objective_activation( event.action );
  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_chest_activation( event.action );
}

void CryptSystem::on_room_event( Events::CryptRoomEvent &event )
{
  if ( Utils::Player::get_mortality( reg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  if ( event.type == Events::CryptRoomEvent::Type::SHUFFLE_PASSAGES ) { shuffle_rooms_passages(); }
  else if ( event.type == Events::CryptRoomEvent::Type::FINAL_PASSAGE ) { unlock_objective_passage(); }
  else if ( event.type == Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) { unlock_exit_passage(); }
}

void CryptSystem::check_entrance_collision()
{
  auto player_pos = Utils::Player::get_position( reg() );
  auto door_view = reg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [door_entity, door_cmp, door_pos_cmp] : door_view.each() )
  {
    // optimize: skip if not visible
    if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), door_pos_cmp ) ) continue;

    // shrink entrance bounds slightly for better UX
    auto decreased_entrance_bounds = Cmp::RectBounds::scaled( door_pos_cmp.position, door_pos_cmp.size, 0.1f, Cmp::RectBounds::ScaleAxis::XY );

    if ( not player_pos.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;
    m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_CRYPT );

    auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
    auto player_pos = Utils::Player::get_position( reg() ).position;
    get_systems_event_queue().trigger( Events::DropInventoryEvent( inventory_entt, player_pos ) );

    Factory::remove_player_last_graveyard_pos( reg() );
    Cmp::Position last_known_pos(
        {
            door_pos_cmp.position.x,
            door_pos_cmp.position.y + Constants::kGridSizePxF.y,
        },
        Constants::kGridSizePxF );
    SPDLOG_INFO( "Last known graveyard position {}, {}", last_known_pos.position.x, last_known_pos.position.y );
    Factory::add_player_last_graveyard_pos( reg(), last_known_pos );
    break;
  }
}

void CryptSystem::check_exit_collision()
{
  auto player_pos = Utils::Player::get_position( reg() );
  auto door_view = reg().view<Cmp::Exit, Cmp::Position>();

  for ( auto [door_entity, door_cmp, door_pos_cmp] : door_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), door_pos_cmp ) ) continue;

    auto decreased_entrance_bounds = Cmp::RectBounds::scaled( door_pos_cmp.position, door_pos_cmp.size, 0.1f,
                                                              Cmp::RectBounds::ScaleAxis::XY ); // shrink entrance bounds slightly for better UX

    if ( not player_pos.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;
    m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_CRYPT );
  }
}

void CryptSystem::unlock_crypt_door()
{

  auto player_pos_cmp = Utils::Player::get_position( reg() );
  auto cryptdoor_view = reg().view<Cmp::CryptEntrance, Cmp::Position>();

  auto [inv_entt, inv_type] = Utils::Player::get_inventory_type( reg() );
  if ( inv_type != "sprite.item.cryptkey" ) return;

  for ( auto [door_entity, cryptdoor_cmp, door_pos_cmp] : cryptdoor_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), door_pos_cmp ) ) continue;

    // Player can't intersect with a closed crypt door so expand their hitbox to facilitate collision detection
    auto player_hitbox = Cmp::RectBounds::scaled( player_pos_cmp, 5.f );
    if ( not player_hitbox.findIntersection( door_pos_cmp ) ) continue;

    // Crypt door is already opened
    if ( cryptdoor_cmp.is_open() )
    {

      // Set the z-order value
      auto crypt_view = reg().view<Cmp::CryptMultiBlock>();
      for ( auto [crypt_entt, crypt_cmp] : crypt_view.each() )
      {
        if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
        reg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entt, crypt_cmp.position.y - 16.f );
        if ( reg().any_of<Cmp::PlayerNoPath>( crypt_entt ) )
        {
          SPDLOG_DEBUG( "CryptDoor is open" );
          reg().remove<Cmp::PlayerNoPath>( crypt_entt );
        }
      }
      continue;
    }
    else
    {
      // Set the z-order value
      auto crypt_view = reg().view<Cmp::CryptMultiBlock>();
      for ( auto [crypt_entt, crypt_cmp] : crypt_view.each() )
      {
        if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
        reg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entt, crypt_cmp.position.y + 16.f );
      }
    }

    // unlock the crypt door
    SPDLOG_DEBUG( "Player unlocked a crypt door at ({}, {})", door_pos_cmp.position.x, door_pos_cmp.position.y );
    Factory::destroy_inventory( reg(), "sprite.item.cryptkey" );

    // player_key_count->decrement_count( 1 );
    m_sound_bank.get_effect( "crypt_open" ).play();
    cryptdoor_cmp.set_is_open( true );
    reg().remove<Cmp::PlayerNoPath>( door_entity );

    // make doorway non-solid and lower z-order so player walks over it
    reg().emplace_or_replace<Cmp::CryptSegment>( door_entity, Cmp::CryptSegment( false ) );

    // find the crypt multi-block this door belongs to and update the sprite
    auto crypt_view = reg().view<Cmp::CryptMultiBlock, Cmp::SpriteAnimation>();
    for ( auto [crypt_entity, crypt_cmp, anim_cmp] : crypt_view.each() )
    {
      if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
      anim_cmp.m_sprite_type = "CRYPT.opened";

      SPDLOG_DEBUG( "Updated crypt multi-block sprite to open state at ({}, {})", crypt_cmp.position.x, crypt_cmp.position.y );
      break;
    }
  }
}

void CryptSystem::check_objective_activation( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PlayerCadaverCount>();
  auto grave_view = reg().view<Cmp::CryptObjectiveMultiBlock>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_cadaver_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );

    for ( auto [objective_entity, objective_cmp] : grave_view.each() )
    {
      // did we already get the cadaver from this objective?
      if ( objective_cmp.get_activation_count() >= objective_cmp.get_activation_threshold() ) continue;

      if ( player_hitbox.findIntersection( objective_cmp ) )
      {
        sf::FloatRect expanded_search( sf::Vector2f{ objective_cmp.position.x, objective_cmp.position.y + objective_cmp.size.y },
                                       sf::Vector2f{ objective_cmp.size.x, Constants::kGridSizePxF.y * 2.f } );
        auto obst_entity = Factory::create_loot_drop( reg(), Cmp::SpriteAnimation{ 0, 0, true, "sprite.crypt.loot.cadaver", 0 }, expanded_search,
                                                      Factory::IncludePack<>{},
                                                      Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::CryptObjectiveSegment>{} );
        if ( obst_entity != entt::null )
        {
          m_sound_bank.get_effect( "drop_loot" ).play();
          objective_cmp.increment_activation_count();
          SPDLOG_INFO( "Player activated crypt objective." );

          const auto &ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_objective_opened" );
          reg().emplace_or_replace<Cmp::SpriteAnimation>( objective_entity, 0, 0, true, ms.get_sprite_type(), 0 );
        }
      }
    }
  }
}

void CryptSystem::check_lever_activation()
{
  if ( m_maze_unlocked ) return;

  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto lever_view = reg().view<Cmp::CryptLever, Cmp::Position>();

  for ( auto [pc_entity, player_cmp, player_pos_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds::scaled( player_pos_cmp.position, Constants::kGridSizePxF, 0.5f );
    for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : lever_view.each() )
    {
      // prevent player from spamming lever twice
      if ( lever_cmp.isEnabled() ) { continue; }
      if ( not player_hitbox.findIntersection( lever_pos_cmp ) ) { continue; }
      lever_cmp.setEnabled( true );
      m_enabled_levers++;

      // update the sprite
      Sprites::SpriteMetaType lever_sprite_type = "sprite.crypt.switch";
      unsigned int enabled_lever_sprite_idx = 1;
      reg().emplace_or_replace<Cmp::SpriteAnimation>( lever_entt, 0, 0, true, lever_sprite_type, enabled_lever_sprite_idx );

      m_sound_bank.get_effect( "crypt_lever_open" ).play();
      SPDLOG_DEBUG( "Lever enabled at {},{} - Count:{}", lever_pos_cmp.position.x, lever_pos_cmp.position.y, m_enabled_levers );

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

void CryptSystem::check_chest_activation( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto chest_view = reg().view<Cmp::CryptChest, Cmp::Position, Cmp::SpriteAnimation>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
    for ( auto [chest_entt, chest_cmp, chest_pos_cmp, chest_anim_cmp] : chest_view.each() )
    {
      // prevent player from spamming chest twice
      if ( chest_cmp.open == true ) continue;
      if ( not player_hitbox.findIntersection( chest_pos_cmp ) ) continue;

      chest_cmp.open = true;
      chest_anim_cmp.m_animation_active = true;
      m_sound_bank.get_effect( "crypt_chest_open" ).play();

      // clang-format off
      auto loot_entt = Factory::create_loot_drop( 
        reg(), 
        Cmp::SpriteAnimation( 0, 0, true, "sprite.crypt.loot.gold", 0 ),                                        
        Cmp::RectBounds::scaled( chest_pos_cmp, 3.f ).getBounds(), 
        Factory::IncludePack<>{},
        Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::CryptChest, Cmp::CryptRoomLavaPitCell, Cmp::CryptPassageBlock, Cmp::Wall, Cmp::Obstacle>{} ,
        Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::CryptChest, Cmp::CryptRoomLavaPitCell, Cmp::CryptPassageBlock, Cmp::Wall, Cmp::Obstacle>{},
        64.f);
      // clang-format on

      if ( loot_entt != entt::null ) m_sound_bank.get_effect( "drop_loot" ).play();
    }
  }
}

void CryptSystem::createRoomBorders()
{
  auto add_borders_for_room = [&]<typename Component>( Component &room_cmp, Sprites::SpriteMetaType sprite_type, size_t sprite_index )
  {
    const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( sprite_type );
    for ( auto &[pos_entt, pos_cmp] : room_cmp.m_border_position_list )
    {
      if ( not reg().valid( pos_entt ) ) continue;
      Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, sprite_index );
      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
        pathfinding_navmesh->remove( pos_entt, pos_cmp );
    }
  };

  for ( auto [closed_room_entt, closed_room_cmp] : reg().view<Cmp::CryptRoomClosed>().each() )
    add_borders_for_room( closed_room_cmp, "sprite.crypt.wall.int", 0 );

  for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
    add_borders_for_room( open_room_cmp, "sprite.crypt.wall.int", 1 );
}

void CryptSystem::shuffle_rooms_passages()
{
  auto selected_rooms = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomClosed>(
      reg(), 4, {}, Utils::Rnd::ExcludePack<Cmp::CryptRoomStart, Cmp::CryptRoomEnd>{}, 0 );

  // reset rooms/passages
  remove_lava_pit_open_rooms();
  remove_lever_open_rooms();
  remove_chest_open_rooms();
  close_open_rooms();
  fill_closed_rooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::REMOVE_PASSAGES ) );

  // open new rooms/passages
  open_selected_rooms( selected_rooms );
  empty_open_rooms();
  createRoomBorders();
  add_lava_pit_open_rooms();

  // try to open passages for the occupied room: only do start room if player is currently there
  SPDLOG_DEBUG( "~~~~~~~~~~~ STARTING PASSAGE GEN ~~~~~~~~~~~~~~~" );
  auto [start_room_entt, start_room_cmp] = get_crypt_room_start();
  if ( Utils::Player::get_position( reg() ).findIntersection( start_room_cmp ) )
  {
    get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, start_room_entt ) );
  }
  else { get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_OPENROOMS ) ); }

  add_chest_to_open_rooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );

  add_lever_to_open_rooms();

  m_sound_bank.get_effect( "crypt_room_shuffle" ).play();
  Scene::CryptScene::get_maze_timer().restart();
}

void CryptSystem::gen_crypt_initial_interior()
{
  SPDLOG_DEBUG( "Generating crypt interior obstacles." );
  auto position_view = reg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  // auto room_view = getReg().view<Cmp::CryptRoomClosed>();
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    // skip if inside a start/end/open room
    bool add_interior_wall = true;
    auto start_room_view = reg().view<Cmp::CryptRoomStart>();
    for ( auto [start_room_entity, start_room_cmp] : start_room_view.each() )
    {
      if ( pos_cmp.findIntersection( start_room_cmp ) ) add_interior_wall = false;
    }
    auto end_room_view = reg().view<Cmp::CryptRoomEnd>();
    for ( auto [end_room_entity, end_room_cmp] : end_room_view.each() )
    {
      if ( pos_cmp.findIntersection( end_room_cmp ) ) add_interior_wall = false;
    }
    auto open_room_view = reg().view<Cmp::CryptRoomOpen>();
    for ( auto [open_room_entity, open_room_cmp] : open_room_view.each() )
    {
      if ( pos_cmp.findIntersection( open_room_cmp ) ) add_interior_wall = false;
    }

    if ( add_interior_wall )
    {
      // if non-zero use the sprites.json zorder value, else use the sprites y-xis pos
      const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "sprite.crypt.wall.int" );

      Factory::create_obstacle( reg(), entity, pos_cmp, ms, 0 );
    }
  }
}

void CryptSystem::create_initial_crypt_rooms( sf::Vector2u map_grid_size )
{
  const auto &grid_square_size = Constants::kGridSizePxF;
  const int min_room_width = 3;
  const int min_room_height = 3;
  const int max_room_width = 8;
  const int max_room_height = 8;
  const int max_distance_between_rooms = 2;
  const int max_attempts = 5000;

  int room_count = 0;
  int current_attempt = 0;
  while ( room_count < 20 )
  {

    int room_width = Cmp::RandomInt{ min_room_width, max_room_width }.gen();
    int room_height = Cmp::RandomInt{ min_room_height, max_room_height }.gen();
    auto [entt, pos] = Utils::Rnd::get_random_position( reg(), {}, Utils::Rnd::ExcludePack<Cmp::ReservedPosition>{}, 0 );
    Cmp::CryptRoomClosed new_room( pos.position, { room_width * grid_square_size.x, room_height * grid_square_size.y } );
    SPDLOG_DEBUG( "Generated new room at ({}, {}) size ({}, {})", new_room.position.x, new_room.position.y, new_room.size.x, new_room.size.y );

    auto is_min_distance_ok = [&]( const Cmp::CryptRoomClosed &existing_room, const Cmp::CryptRoomClosed &new_room ) -> bool
    {
      float distance_x = std::max( 0.f, std::max( existing_room.position.x - ( new_room.position.x + new_room.size.x ),
                                                  new_room.position.x - ( existing_room.position.x + existing_room.size.x ) ) );
      float distance_y = std::max( 0.f, std::max( existing_room.position.y - ( new_room.position.y + new_room.size.y ),
                                                  new_room.position.y - ( existing_room.position.y + existing_room.size.y ) ) );
      float distance = std::sqrt( distance_x * distance_x + distance_y * distance_y );
      return distance >= static_cast<float>( max_distance_between_rooms ) * grid_square_size.x;
    };

    auto check_collision = [&]( const auto &existing_object ) -> bool
    { return new_room.findIntersection( existing_object ) || !is_min_distance_ok( existing_object, new_room ); };

    bool overlaps_existing = false;

    // make sure new_room area does not fall outside map_grid_size
    if ( !Utils::isInBounds( new_room.position, new_room.size, map_grid_size ) ) { overlaps_existing = true; }

    // check for intersection with existing rooms
    if ( !overlaps_existing )
    {
      auto room_view = reg().view<Cmp::CryptRoomClosed>();
      for ( auto [existing_entity, existing_room] : room_view.each() )
      {
        if ( check_collision( existing_room ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with walls
    if ( !overlaps_existing )
    {
      auto wall_view = reg().view<Cmp::Wall, Cmp::Position>();
      for ( auto [wall_entity, wall_cmp, wall_pos_cmp] : wall_view.each() )
      {
        if ( check_collision( wall_pos_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with start room
    if ( !overlaps_existing )
    {
      auto start_room_view = reg().view<Cmp::CryptRoomStart>();
      for ( auto [start_room_entity, start_room_cmp] : start_room_view.each() )
      {
        if ( check_collision( start_room_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with end room
    if ( !overlaps_existing )
    {
      auto end_room_view = reg().view<Cmp::CryptRoomEnd>();
      for ( auto [end_room_entity, end_room_cmp] : end_room_view.each() )
      {
        if ( check_collision( end_room_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // now check the result
    if ( !overlaps_existing )
    {
      auto entity = reg().create();

      float room_left = new_room.position.x;
      float room_right = new_room.position.x + new_room.size.x;
      float room_top = new_room.position.y;
      float room_bottom = new_room.position.y + new_room.size.y;

      sf::FloatRect border_area(
          sf::Vector2f{ room_left - Constants::kGridSizePxF.x, room_top - Constants::kGridSizePxF.y },
          sf::Vector2f{ new_room.size.x + Constants::kGridSizePxF.x * 2.f, new_room.size.y + Constants::kGridSizePxF.y * 2.f } );

      for ( auto [pos_entt, pos_cmp] : reg().view<Cmp::Position>().each() )
      {
        if ( reg().any_of<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Direction>( pos_entt ) ) continue;
        if ( reg().any_of<Cmp::Wall, Cmp::Exit>( pos_entt ) ) continue;

        // interior positions
        if ( pos_cmp.findIntersection( new_room ) ) { new_room.m_position_list.push_back( { pos_entt, pos_cmp } ); }
        // border positions (1-tile ring outside the room, excluding interior)
        else if ( border_area.contains( pos_cmp.position ) )
        {
          bool is_border = false;
          if ( pos_cmp.position.x == room_left - Constants::kGridSizePxF.x && pos_cmp.position.y >= room_top - Constants::kGridSizePxF.y &&
               pos_cmp.position.y <= room_bottom )
            is_border = true;
          else if ( pos_cmp.position.x == room_right && pos_cmp.position.y >= room_top - Constants::kGridSizePxF.y &&
                    pos_cmp.position.y <= room_bottom )
            is_border = true;
          else if ( pos_cmp.position.y == room_top - Constants::kGridSizePxF.y && pos_cmp.position.x >= room_left - Constants::kGridSizePxF.x &&
                    pos_cmp.position.x <= room_right )
            is_border = true;
          else if ( pos_cmp.position.y == room_bottom && pos_cmp.position.x >= room_left - Constants::kGridSizePxF.x &&
                    pos_cmp.position.x <= room_right )
            is_border = true;
          if ( is_border ) new_room.m_border_position_list.push_back( { pos_entt, pos_cmp } );
        }
      }

      SPDLOG_DEBUG( "Added new crypt room entity {}, total rooms: {}, position count {}", entt::to_integral( entity ), room_count,
                    new_room.m_position_list.size() );
      reg().emplace<Cmp::CryptRoomClosed>( entity, std::move( new_room ) );
      room_count++;
    }
    else
    {
      SPDLOG_DEBUG( "New room overlaps existing room, discarded. ({}/{})", current_attempt, max_attempts );
      current_attempt++;
    }
    if ( current_attempt >= max_attempts )
    {
      SPDLOG_WARN( "Max attempts reached when generating crypt rooms, stopping." );
      return;
    }
  }

  SPDLOG_DEBUG( "Total rooms: {}", room_count );
  // Currently no special logic needed; placeholder for future use
}

void CryptSystem::cache_all_room_connections()
{
  SPDLOG_DEBUG( "CryptSystem::cache_all_room_connections()" );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CACHE_ALL_ROOM_CONNECTIONS ) );
}

void CryptSystem::create_end_room( sf::Vector2u map_grid_size )
{
  auto map_grid_sizef = sf::Vector2f( static_cast<float>( map_grid_size.x ) * Constants::kGridSizePxF.x,
                                      static_cast<float>( map_grid_size.y ) * Constants::kGridSizePxF.y );
  auto kGridSizePxF = Constants::kGridSizePxF;
  // target position for the objective: always center top of the map
  const auto &ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_objective_closed" );

  float centered_x = ( map_grid_sizef.x / 2.f ) - ( ms.getSpriteSizePixels().x / 2.f );
  Cmp::Position objective_position( { centered_x, kGridSizePxF.y * 2.f }, ms.getSpriteSizePixels() );

  auto end_room_entity = reg().create();
  reg().emplace_or_replace<Cmp::CryptRoomEnd>( end_room_entity,
                                               sf::Vector2f{ objective_position.position.x - kGridSizePxF.x, objective_position.position.y },
                                               sf::Vector2f{ objective_position.size.x, objective_position.size.y + ( kGridSizePxF.y * 2.f ) } );
}

/// PRIVATE FUNCTIONS
void CryptSystem::unlock_objective_passage()
{
  // reset rooms/passages
  remove_lava_pit_open_rooms();
  remove_lever_open_rooms();
  remove_chest_open_rooms();
  close_open_rooms();
  remove_all_levers();
  fill_closed_rooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::REMOVE_PASSAGES ) );
  createRoomBorders();

  // open new rooms/passages
  SPDLOG_DEBUG( "~~~~~~~~~~~ OPENING FINAL PASSAGE ~~~~~~~~~~~~~~~" );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_ENDROOM, get_crypt_room_end().first ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );
  m_sound_bank.get_effect( "crypt_room_shuffle" ).play();

  // switch on the lights so we can see the objective in all its glory!
  for ( auto [entt, sys_cmp] : reg().view<Cmp::System>().each() )
  {
    sys_cmp.dark_mode_enabled = false;
  }
}

void CryptSystem::unlock_exit_passage()
{
  // if we unlocked the maze by picking up the cadaver, then cancel the timer
  Scene::CryptScene::get_maze_timer().reset();
  m_maze_unlocked = true;

  close_open_rooms();
  remove_all_chests();
  remove_all_levers();
  remove_lava_pit_open_rooms();
  open_all_rooms();
  empty_open_rooms();
  createRoomBorders();

  // make sure player can reach exit
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, get_crypt_room_start().first ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_ALL_ROOMS ) );

  spawn_npc_in_open_rooms();
}

void CryptSystem::close_open_rooms()
{

  std::vector<std::pair<entt::entity, Cmp::CryptRoomOpen>> rooms_to_close;
  auto open_room_view = reg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( open_room_cmp.findIntersection( Utils::Player::get_position( reg() ) ) ) continue;
    rooms_to_close.push_back( { open_room_entt, open_room_cmp } );
  }

  // close the entities safely outside of view
  for ( auto [room_entt, open_room_cmp] : rooms_to_close )
  {
    // transfer the position list to the new open room before destroying the closed room
    Cmp::CryptRoomClosed new_closed_room( open_room_cmp.position, open_room_cmp.size );
    new_closed_room.m_position_list = open_room_cmp.m_position_list;
    new_closed_room.m_border_position_list = open_room_cmp.m_border_position_list;
    reg().emplace<Cmp::CryptRoomClosed>( room_entt, std::move( new_closed_room ) );
    reg().remove<Cmp::CryptRoomOpen>( room_entt );
  }
}

void CryptSystem::fill_closed_rooms()
{

  for ( auto [closed_room_entt, closed_room_cmp] : reg().view<Cmp::CryptRoomClosed>().each() )
  {
    for ( auto [pos_entt, pos_cmp] : closed_room_cmp.m_position_list )
    {
      // skip position outside closed room area or if position already has existing obstacle
      // if ( not closed_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( reg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;
      if ( reg().any_of<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Direction>( pos_entt ) ) continue;

      const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "sprite.crypt.wall.int" );
      Factory::create_obstacle( reg(), pos_entt, pos_cmp, ms, 0 );

      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
      {
        pathfinding_navmesh->remove( pos_entt, pos_cmp );
      }
    }
  }
}

void CryptSystem::open_selected_rooms( std::set<entt::entity> selected_rooms )
{
  std::vector<std::pair<entt::entity, Cmp::CryptRoomClosed>> rooms_to_open;
  auto closed_room_view = reg().view<Cmp::CryptRoomClosed>();
  for ( auto [closed_room_entt, closed_room_cmp] : closed_room_view.each() )
  {
    if ( selected_rooms.find( closed_room_entt ) == selected_rooms.end() ) continue;

    // save the CryptRoomClosed entities we want to change to CryptRoomOpen
    rooms_to_open.push_back( { closed_room_entt, closed_room_cmp } );
  }

  // open the room safely outside of view loop
  for ( auto [room_entt, closed_room_cmp] : rooms_to_open )
  {
    // transfer the position list to the new open room before destroying the closed room
    Cmp::CryptRoomOpen new_open_room( closed_room_cmp.position, closed_room_cmp.size );
    new_open_room.m_position_list = closed_room_cmp.m_position_list;
    new_open_room.m_border_position_list = closed_room_cmp.m_border_position_list;
    reg().emplace<Cmp::CryptRoomOpen>( room_entt, std::move( new_open_room ) );
    reg().remove<Cmp::CryptRoomClosed>( room_entt );
  }
}

void CryptSystem::open_all_rooms()
{
  std::set<entt::entity> all_rooms;
  for ( auto [closed_room_entt, closed_room_cmp] : reg().view<Cmp::CryptRoomClosed>().each() )
  {
    all_rooms.insert( closed_room_entt );
  }
  open_selected_rooms( all_rooms );
}

void CryptSystem::empty_open_rooms()
{

  for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
  {
    for ( auto [pos_entt, pos_cmp] : open_room_cmp.m_position_list )
    {
      // skip position outside open room area or if position doesn't have existing obstacle
      if ( not open_room_cmp.findIntersection( pos_cmp ) ) continue;
      if ( not reg().all_of<Cmp::Obstacle>( pos_entt ) ) continue;

      Factory::remove_obstacle( reg(), pos_entt );
      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
      {
        pathfinding_navmesh->insert( pos_entt, pos_cmp );
      }
    }
  }
}

void CryptSystem::add_lava_pit_open_rooms()
{
  auto open_room_view = reg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
    {
      Factory::create_crypt_lava_pit( reg(), open_room_cmp, pathfinding_navmesh );
    }
  }
}

void CryptSystem::do_lava_pit_animation()
{
  // remove any pre-existing lava anim entities
  auto lava_anim_view = reg().view<Cmp::CryptRoomLavaPitCellEffect, Cmp::SpriteAnimation>();
  for ( auto [lava_anim_entt, lava_cell_anim_cmp, lava_anim_cmp] : lava_anim_view.each() )
  {
    // only delete the entity if it has finished its animation sequence
    if ( lava_anim_cmp.m_animation_active == true ) continue;
    if ( reg().valid( lava_anim_entt ) ) reg().destroy( lava_anim_entt );
  }

  if ( m_lava_effect_cooldown_timer.getElapsedTime() > m_lava_effect_cooldown_threshold )
  {
    // add N new lava anim entities
    auto found_lava_cell_entts = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomLavaPitCell>( reg(), 1, {}, {} );

    for ( auto lava_cell_entt : found_lava_cell_entts )
    {
      // disabled CryptRoomLavaPitCells have no zorder
      auto lava_zorder_cmp = reg().try_get<Cmp::ZOrderValue>( lava_cell_entt );
      if ( not lava_zorder_cmp ) continue;

      auto lava_cell_cmp = reg().try_get<Cmp::CryptRoomLavaPitCell>( lava_cell_entt );
      if ( not lava_cell_cmp ) continue;

      auto lava_anim_entt = reg().create();

      reg().emplace_or_replace<Cmp::CryptRoomLavaPitCellEffect>( lava_anim_entt );
      reg().emplace_or_replace<Cmp::Position>( lava_anim_entt, lava_cell_cmp->position, lava_cell_cmp->size );
      reg().emplace_or_replace<Cmp::SpriteAnimation>( lava_anim_entt, 0, 0, true, "CRYPT.lava_anim", 0, 0.2, Cmp::AnimType::ONESHOTRESET );
      reg().emplace_or_replace<Cmp::ZOrderValue>( lava_anim_entt, lava_cell_cmp->position.y + 64.f );
    }
    m_lava_effect_cooldown_timer.restart();
  }
}

void CryptSystem::remove_lava_pit_open_rooms()
{
  auto player_pos_cmp = Utils::Player::get_position( reg() );
  auto open_room_view = reg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( open_room_cmp.findIntersection( player_pos_cmp ) ) continue; // skip occupied rooms
    auto lava_pit_view = reg().view<Cmp::CryptRoomLavaPit>();
    for ( auto [lava_pit_entt, lava_pit_cmp] : lava_pit_view.each() )
    {
      if ( not open_room_cmp.findIntersection( lava_pit_cmp ) ) continue;
      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
      {
        Factory::destroy_crypt_lava_pit( reg(), lava_pit_entt, pathfinding_navmesh );
      }
    }
  }
}

void CryptSystem::check_lava_pit_collision()
{
  if ( Utils::Player::get_mortality( reg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  auto player_hitbox = Cmp::RectBounds::scaled( Utils::Player::get_position( reg() ).position, Utils::Player::get_position( reg() ).size, 0.5f );
  for ( auto [lava_cell_entt, lava_cell_cmp] : reg().view<Cmp::CryptRoomLavaPitCell>().each() )
  {
    if ( not player_hitbox.findIntersection( lava_cell_cmp ) ) continue;
    Scene::CryptScene::stop_maze_timer();
    get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::IGNITED, lava_cell_cmp ) );
  }
}

void CryptSystem::check_lava_pit_activation_by_proximity()
{
  auto player_pos_cmp = Utils::Player::get_position( reg() );
  auto player_hitbox_enable = Cmp::RectBounds::scaled( player_pos_cmp.position, player_pos_cmp.size, 2.f );
  auto player_hitbox_disable = Cmp::RectBounds::scaled( player_pos_cmp.position, player_pos_cmp.size, 5.f );
  int num_lava_pits_intersect = 0;
  for ( auto [lava_pit_entt, lava_pit_cmp] : reg().view<Cmp::CryptRoomLavaPit>().each() )
  {
    if ( player_hitbox_enable.findIntersection( lava_pit_cmp ) ) num_lava_pits_intersect++;
    for ( auto [lava_cell_entt, lava_cell_cmp] : reg().view<Cmp::CryptRoomLavaPitCell>().each() )
    {

      if ( not lava_cell_cmp.findIntersection( lava_pit_cmp ) ) continue;
      if ( player_hitbox_enable.findIntersection( lava_pit_cmp ) )
      {
        // enable rendering of this cell by adding a zorder component
        reg().emplace_or_replace<Cmp::ZOrderValue>( lava_cell_entt, lava_cell_cmp.position.y );
      }
      else if ( not player_hitbox_disable.findIntersection( lava_pit_cmp ) )
      {
        // disable the rendering of this cell by removing its zorder component
        reg().remove<Cmp::ZOrderValue>( lava_cell_entt );
      }
    }
  }

  // finally play/stop sound if any/all lava pits activated/deactivated
  if ( num_lava_pits_intersect > 0 )
  {
    auto sfx_status = m_sound_bank.get_effect( "bubbling_lava" ).getStatus();
    if ( sfx_status != sf::Sound::Status::Playing ) m_sound_bank.get_effect( "bubbling_lava" ).play();
  }
  else
  {
    //
    m_sound_bank.get_effect( "bubbling_lava" ).stop();
  }
}

void CryptSystem::check_spike_trap_collision()
{
  if ( Utils::Player::get_mortality( reg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  Cmp::Position player_pos = Utils::Player::get_position( reg() );
  auto player_hitbox = Cmp::RectBounds::scaled( player_pos.position, player_pos.size, 0.5f );
  for ( auto [spike_trap_entt, spike_trap_cmp, spike_trap_anim_cmp] : reg().view<Cmp::CryptPassageSpikeTrap, Cmp::SpriteAnimation>().each() )
  {
    if ( not spike_trap_anim_cmp.m_animation_active ) continue;

    Cmp::Position spike_trap_hitbox( spike_trap_cmp, Constants::kGridSizePxF );
    if ( not player_hitbox.findIntersection( spike_trap_hitbox ) ) continue;
    Scene::CryptScene::stop_maze_timer();
    get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SKEWERED, spike_trap_hitbox ) );
  }
}

void CryptSystem::check_spike_trap_activation_by_proximity()
{
  auto player_pos_cmp = Utils::Player::get_position( reg() );
  auto player_hitbox_enable = Cmp::RectBounds::scaled( player_pos_cmp.position, player_pos_cmp.size, 2.f );
  auto player_hitbox_disable = Cmp::RectBounds::scaled( player_pos_cmp.position, player_pos_cmp.size, 5.f );
  for ( auto [spike_trap_entt, spike_trap_cmp, spike_trap_anim_cmp] : reg().view<Cmp::CryptPassageSpikeTrap, Cmp::SpriteAnimation>().each() )
  {
    auto spike_trap_hitbox = sf::FloatRect( spike_trap_cmp, Constants::kGridSizePxF );
    if ( player_hitbox_enable.findIntersection( spike_trap_hitbox ) and
         spike_trap_cmp.m_cooldown_timer.getElapsedTime() > spike_trap_cmp.m_cooldown_threshold )
    {
      // re-activate the threat
      SPDLOG_DEBUG( "Reactivating spike: {}", static_cast<int>( spike_trap_entt ) );
      spike_trap_anim_cmp.m_animation_active = true;
      spike_trap_cmp.m_cooldown_timer.reset();

      auto sfx_status = m_sound_bank.get_effect( "spike_trap" ).getStatus();
      if ( sfx_status != sf::Sound::Status::Playing ) { m_sound_bank.get_effect( "spike_trap" ).play(); }
    }
    else if ( not player_hitbox_disable.findIntersection( spike_trap_hitbox ) )
    {
      // de-activate the threat
      spike_trap_anim_cmp.m_animation_active = false;
      spike_trap_anim_cmp.m_current_frame = spike_trap_anim_cmp.m_base_frame;
      spike_trap_cmp.m_cooldown_timer.restart();
    }
  }
}

std::vector<entt::entity> CryptSystem::get_available_room_positions()
{
  std::vector<entt::entity> internal_room_entts;

  for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
  {
    if ( Utils::Player::get_position( reg() ).findIntersection( open_room_cmp ) ) continue;

    // use pre-built position list instead of iterating all positions
    for ( auto &[pos_entt, pos_cmp] : open_room_cmp.m_position_list )
    {
      if ( not reg().valid( pos_entt ) ) continue;

      bool intersects_lava = false;
      for ( auto [lava_pit_entt, lava_pit_cmp] : reg().view<Cmp::CryptRoomLavaPit>().each() )
      {
        if ( pos_cmp.findIntersection( lava_pit_cmp ) )
        {
          intersects_lava = true;
          break;
        }
      }
      if ( intersects_lava ) continue;

      bool intersects_lever = false;
      for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : reg().view<Cmp::CryptLever, Cmp::Position>().each() )
      {
        if ( pos_cmp.findIntersection( lever_pos_cmp ) )
        {
          intersects_lever = true;
          break;
        }
      }
      if ( intersects_lever ) continue;

      bool intersects_chest = false;
      for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : reg().view<Cmp::CryptChest, Cmp::Position>().each() )
      {
        if ( pos_cmp.findIntersection( chest_pos_cmp ) )
        {
          intersects_chest = true;
          break;
        }
      }
      if ( intersects_chest ) continue;

      bool intersects_passageblock = false;
      for ( auto [pblock_entt, pblock_cmp] : reg().view<Cmp::CryptPassageBlock>().each() )
      {
        auto expanded_hitbox = Cmp::RectBounds::scaled( pos_cmp.position, pos_cmp.size, 4.f );
        if ( expanded_hitbox.findIntersection( sf::FloatRect( pblock_cmp, Constants::kGridSizePxF ) ) )
        {
          intersects_passageblock = true;
          break;
        }
      }
      if ( intersects_passageblock ) continue;

      internal_room_entts.push_back( pos_entt );
    }
  }

  return internal_room_entts;
}

void CryptSystem::add_chest_to_open_rooms()
{
  Sprites::SpriteMetaType chest_sprite_type = "sprite.crypt.chest";
  float zorder = m_sprite_factory.get_sprite_size_by_type( chest_sprite_type ).y;

  // iterate all open rooms
  for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
  {
    if ( open_room_cmp.findIntersection( Utils::Player::get_position( reg() ) ) ) continue;
    // pick a random border position
    Cmp::RandomInt room_border_picker( 0, open_room_cmp.m_border_position_list.size() - 1 );
    auto [selected_entt, selected_pos] = open_room_cmp.m_border_position_list[room_border_picker.gen()];
    Factory::create_crypt_chest( reg(), selected_pos.position, chest_sprite_type, 0, zorder );
    SPDLOG_DEBUG( "Added chest to position: {},{}", selected_pos.position.x, selected_pos.position.y );
  }
}

void CryptSystem::add_lever_to_open_rooms()
{
  auto internal_room_entts = get_available_room_positions();
  Sprites::SpriteMetaType lever_sprite_type = "sprite.crypt.switch";
  unsigned int disabled_lever_sprite_idx = 0;
  float zorder = m_sprite_factory.get_sprite_size_by_type( lever_sprite_type ).y;

  // add one lever to one room picked from the pool of candidates room positions
  Cmp::RandomInt room_position_picker( 0, internal_room_entts.size() - 1 );
  auto selected_entt = internal_room_entts[room_position_picker.gen()];
  auto room_pos = reg().get<Cmp::Position>( selected_entt );
  Factory::create_crypt_lever( reg(), room_pos.position, lever_sprite_type, disabled_lever_sprite_idx, zorder );
  SPDLOG_DEBUG( "Added lever to position: {},{}", room_pos.position.x, room_pos.position.y );
}

void CryptSystem::remove_lever_open_rooms()
{
  for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : reg().view<Cmp::CryptLever, Cmp::Position>().each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( not open_room_cmp.findIntersection( lever_pos_cmp ) ) continue;
      if ( open_room_cmp.findIntersection( Utils::Player::get_position( reg() ) ) ) continue;

      Factory::destroy_crypt_lever( reg(), lever_entt );
    }
  }
}

void CryptSystem::remove_chest_open_rooms()
{
  auto player_pos = Utils::Player::get_position( reg() );

  for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : reg().view<Cmp::CryptChest, Cmp::Position>().each() )
  {
    bool player_in_same_room = false;
    for ( auto [open_room_entt, open_room_cmp] : reg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( not open_room_cmp.findIntersection( player_pos ) ) continue; // player not in this room

      // expand room bounds by 1 tile to include border positions where chests are placed
      // sf::FloatRect expanded_room(
      //     sf::Vector2f{ open_room_cmp.position.x - Constants::kGridSizePxF.x, open_room_cmp.position.y - Constants::kGridSizePxF.y },
      //     sf::Vector2f{ open_room_cmp.size.x + Constants::kGridSizePxF.x * 2.f, open_room_cmp.size.y + Constants::kGridSizePxF.y * 2.f } );

      auto expanded_room = Cmp::RectBounds::expanded( open_room_cmp, 1 );

      if ( expanded_room.findIntersection( chest_pos_cmp ) )
      {
        player_in_same_room = true;
        break;
      }
    }

    if ( not player_in_same_room ) { Factory::destroy_crypt_chest( reg(), chest_entt ); }
  }
}

void CryptSystem::remove_all_levers()
{
  for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : reg().view<Cmp::CryptLever, Cmp::Position>().each() )

  {
    Factory::destroy_crypt_lever( reg(), lever_entt );
  }
}

void CryptSystem::remove_all_chests()
{
  for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : reg().view<Cmp::CryptChest, Cmp::Position>().each() )
  {
    Factory::destroy_crypt_chest( reg(), chest_entt );
  }
}

void CryptSystem::spawn_npc_in_open_rooms()
{
  auto open_room_view = reg().view<Cmp::CryptRoomOpen>();
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
  const int npcs_to_spawn = std::min( Sys::PersistSystem::get<Cmp::Persist::CryptNpcSpawnCount>( reg() ).get_value(),
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
    auto position_entity = reg().create();
    Cmp::Position position_cmp = reg().emplace<Cmp::Position>( position_entity, spawn_position, Constants::kGridSizePxF );
    [[maybe_unused]] Cmp::ZOrderValue zorder_cmp = reg().emplace<Cmp::ZOrderValue>( position_entity, position_cmp.position.y );
    Factory::create_npc( reg(), position_entity, "npc.priest" );

    SPDLOG_DEBUG( "Spawned NPC {} at position ({}, {})", r + 1, spawn_position.x, spawn_position.y );
  }
}

} // namespace ProceduralMaze::Sys