#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptExit.hpp>
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
#include <Components/Inventory/CarryItem.hpp>
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
#include <Utils/Random.hpp>
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

  // check collisions with lava pit
  if ( Utils::getSystemCmp( getReg() ).collisions_enabled )
  {
    checkLavaPitCollision();
    checkSpikeTrapCollision();
    check_lever_activation();
  }
  checkLavaPitActivationByProximity();
  doLavaPitAnimation();
  checkSpikeTrapActivationByProximity();
}

void CryptSystem::on_player_action( Events::PlayerActionEvent &event )
{
  if ( Utils::get_player_mortality( getReg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_objective_activation( event.action );
  if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_chest_activation( event.action );
}

void CryptSystem::on_room_event( Events::CryptRoomEvent &event )
{
  if ( Utils::get_player_mortality( getReg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  if ( event.type == Events::CryptRoomEvent::Type::SHUFFLE_PASSAGES ) { shuffle_rooms_passages(); }
  else if ( event.type == Events::CryptRoomEvent::Type::FINAL_PASSAGE ) { unlock_objective_passage(); }
  else if ( event.type == Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) { unlock_exit_passage(); }
}

void CryptSystem::check_entrance_collision()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, crypt_door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), crypt_door_pos_cmp ) ) continue;

      // shrink entrance bounds slightly for better UX
      Cmp::RectBounds decreased_entrance_bounds( crypt_door_pos_cmp.position, crypt_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH );

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_entrance_collision: Player entering crypt from graveyard at position ({}, {})", pc_pos_cmp.position.x,
                   pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_CRYPT );

      // remember player position
      auto last_player_pos = Factory::add_player_last_graveyard_pos( getReg(), crypt_door_pos_cmp );
      // drop any inventory
      auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
      auto dropped_entt = Factory::dropInventorySlotIntoWorld( getReg(), last_player_pos,
                                                               m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
      if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
    }
  }
}

void CryptSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
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
  getReg().emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  getReg().emplace_or_replace<Cmp::CryptExit>( entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

void CryptSystem::unlock_crypt_door()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  auto [inv_entt, inv_type] = Utils::get_player_inventory_type( getReg() );
  if ( inv_type != "CARRYITEM.cryptkey" ) return;

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

      // Player can't intersect with a closed crypt door so expand their hitbox to facilitate collision detection
      auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, pc_pos_cmp.size, 5.f );
      if ( not player_hitbox.findIntersection( door_pos_cmp ) ) continue;

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

      // unlock the crypt door
      SPDLOG_INFO( "Player unlocked a crypt door at ({}, {})", door_pos_cmp.position.x, door_pos_cmp.position.y );
      Factory::destroyInventory( getReg(), "CARRYITEM.cryptkey" );

      // player_key_count->decrement_count( 1 );
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

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PlayerCadaverCount>();
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
                                                    Factory::ExcludePack<Cmp::PlayerCharacter, Cmp::CryptObjectiveSegment>{} );
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

void CryptSystem::check_lever_activation()
{
  if ( m_maze_unlocked ) return;

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto lever_view = getReg().view<Cmp::CryptLever, Cmp::Position>();

  for ( auto [pc_entity, player_cmp, player_pos_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( player_pos_cmp.position, Constants::kGridSquareSizePixelsF, 0.5f );
    for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : lever_view.each() )
    {
      // prevent player from spamming lever twice
      if ( lever_cmp.isEnabled() ) { continue; }
      if ( not player_hitbox.findIntersection( lever_pos_cmp ) ) { continue; }
      lever_cmp.setEnabled( true );
      m_enabled_levers++;

      // update the sprite
      Sprites::SpriteMetaType lever_sprite_type = "CRYPT.interior_lever";
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

void CryptSystem::check_chest_activation( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto chest_view = getReg().view<Cmp::CryptChest, Cmp::Position, Cmp::SpriteAnimation>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );
    for ( auto [chest_entt, chest_cmp, chest_pos_cmp, chest_anim_cmp] : chest_view.each() )
    {
      // prevent player from spamming chest twice
      if ( chest_cmp.open == true ) continue;
      if ( not player_hitbox.findIntersection( chest_pos_cmp ) ) continue;

      chest_cmp.open = true;
      chest_anim_cmp.m_animation_active = true;
      m_sound_bank.get_effect( "crypt_chest_open" ).play();

      // clang-format off
      auto loot_entt = Factory::createLootDrop( 
        getReg(), 
        Cmp::SpriteAnimation( 0, 0, true, "LOOT.goldcoin", 0 ),                                        
        Cmp::RectBounds{ chest_pos_cmp.position, chest_pos_cmp.size, 3.f }.getBounds(), 
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
  removeLavaPitOpenRooms();
  removeLeverOpenRooms();
  removeChestOpenRooms();
  closeOpenRooms();
  fillClosedRooms();
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::REMOVE_PASSAGES ) );

  // open new rooms/passages
  openSelectedRooms( selected_rooms );
  emptyOpenRooms();
  createRoomBorders();
  addLavaPitOpenRooms();

  // try to open passages for the occupied room: only do start room if player is currently there
  SPDLOG_INFO( "~~~~~~~~~~~ STARTING PASSAGE GEN ~~~~~~~~~~~~~~~" );
  auto [start_room_entt, start_room_cmp] = get_crypt_room_start();
  if ( Utils::get_player_position( getReg() ).findIntersection( start_room_cmp ) )
  {
    get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, start_room_entt ) );
  }
  else { get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_OCCUPIED_TO_OPENROOMS ) ); }

  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );

  addLeverToOpenRooms();
  addChestToOpenRooms();

  m_sound_bank.get_effect( "crypt_room_shuffle" ).play();
  Scene::CryptScene::get_maze_timer().restart();
}

void CryptSystem::unlock_objective_passage()
{
  // reset rooms/passages
  removeLavaPitOpenRooms();
  removeLeverOpenRooms();
  removeChestOpenRooms();
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

  // switch on the lights so we can see the objective in all its glory!
  for ( auto [entt, sys_cmp] : getReg().view<Cmp::System>().each() )
  {
    sys_cmp.dark_mode_enabled = false;
  }
}

void CryptSystem::unlock_exit_passage()
{
  // if we unlocked the maze by picking up the cadaver, then cancel the timer
  Scene::CryptScene::get_maze_timer().reset();
  m_maze_unlocked = true;

  closeOpenRooms();
  removeAllChests();
  removeAllLevers();
  removeLavaPitOpenRooms();
  openAllRooms();
  emptyOpenRooms();
  createRoomBorders();

  // make sure player can reach exit
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_START_TO_OPENROOMS, get_crypt_room_start().first ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::CONNECT_ALL_OPENROOMS ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::OPEN_PASSAGES ) );
  get_systems_event_queue().trigger( Events::PassageEvent( Events::PassageEvent::Type::ADD_SPIKE_TRAPS ) );

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

void CryptSystem::addLavaPitOpenRooms()
{
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    Factory::createCryptLavaPit( getReg(), open_room_cmp );
  }
}

void CryptSystem::doLavaPitAnimation()
{
  // remove any pre-existing lava anim entities
  auto lava_anim_view = getReg().view<Cmp::CryptRoomLavaPitCellEffect, Cmp::SpriteAnimation>();
  for ( auto [lava_anim_entt, lava_cell_anim_cmp, lava_anim_cmp] : lava_anim_view.each() )
  {
    // only delete the entity if it has finished its animation sequence
    if ( lava_anim_cmp.m_animation_active == true ) continue;
    if ( getReg().valid( lava_anim_entt ) ) getReg().destroy( lava_anim_entt );
  }

  if ( m_lava_effect_cooldown_timer.getElapsedTime() > m_lava_effect_cooldown_threshold )
  {
    // add N new lava anim entities
    auto found_lava_cell_entts = Utils::Rnd::get_n_rand_components<Cmp::CryptRoomLavaPitCell>( getReg(), 1, {}, {} );

    for ( auto lava_cell_entt : found_lava_cell_entts )
    {
      // disabled CryptRoomLavaPitCells have no zorder
      auto lava_zorder_cmp = getReg().try_get<Cmp::ZOrderValue>( lava_cell_entt );
      if ( not lava_zorder_cmp ) continue;

      auto lava_cell_cmp = getReg().try_get<Cmp::CryptRoomLavaPitCell>( lava_cell_entt );
      if ( not lava_cell_cmp ) continue;

      auto lava_anim_entt = getReg().create();

      getReg().emplace_or_replace<Cmp::CryptRoomLavaPitCellEffect>( lava_anim_entt );
      getReg().emplace_or_replace<Cmp::Position>( lava_anim_entt, lava_cell_cmp->position, lava_cell_cmp->size );
      getReg().emplace_or_replace<Cmp::SpriteAnimation>( lava_anim_entt, 0, 0, true, "CRYPT.lava_anim", 0 );
      getReg().emplace_or_replace<Cmp::ZOrderValue>( lava_anim_entt, lava_cell_cmp->position.y + 64.f );
    }
    m_lava_effect_cooldown_timer.restart();
  }
}

void CryptSystem::removeLavaPitOpenRooms()
{
  auto player_pos_cmp = Utils::get_player_position( getReg() );
  auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
  for ( auto [open_room_entt, open_room_cmp] : open_room_view.each() )
  {
    if ( open_room_cmp.findIntersection( player_pos_cmp ) ) continue; // skip occupied rooms
    auto lava_pit_view = getReg().view<Cmp::CryptRoomLavaPit>();
    for ( auto [lava_pit_entt, lava_pit_cmp] : lava_pit_view.each() )
    {
      if ( not open_room_cmp.findIntersection( lava_pit_cmp ) ) continue;
      Factory::destroyCryptLavaPit( getReg(), lava_pit_entt );
    }
  }
}

void CryptSystem::checkLavaPitCollision()
{
  if ( Utils::get_player_mortality( getReg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  Cmp::RectBounds player_hitbox( Utils::get_player_position( getReg() ).position, Utils::get_player_position( getReg() ).size, 0.5f );
  for ( auto [lava_cell_entt, lava_cell_cmp] : getReg().view<Cmp::CryptRoomLavaPitCell>().each() )
  {
    if ( not player_hitbox.findIntersection( lava_cell_cmp ) ) continue;
    Scene::CryptScene::stop_maze_timer();
    get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::IGNITED, lava_cell_cmp ) );
  }
}

void CryptSystem::checkLavaPitActivationByProximity()
{
  auto player_pos_cmp = Utils::get_player_position( getReg() );
  Cmp::RectBounds player_hitbox_enable( player_pos_cmp.position, player_pos_cmp.size, 2.f );
  Cmp::RectBounds player_hitbox_disable( player_pos_cmp.position, player_pos_cmp.size, 5.f );
  int num_lava_pits_intersect = 0;
  for ( auto [lava_pit_entt, lava_pit_cmp] : getReg().view<Cmp::CryptRoomLavaPit>().each() )
  {
    if ( player_hitbox_enable.findIntersection( lava_pit_cmp ) ) num_lava_pits_intersect++;
    for ( auto [lava_cell_entt, lava_cell_cmp] : getReg().view<Cmp::CryptRoomLavaPitCell>().each() )
    {

      if ( not lava_cell_cmp.findIntersection( lava_pit_cmp ) ) continue;
      if ( player_hitbox_enable.findIntersection( lava_pit_cmp ) )
      {
        // enable rendering of this cell by adding a zorder component
        getReg().emplace_or_replace<Cmp::ZOrderValue>( lava_cell_entt, lava_cell_cmp.position.y );
      }
      else if ( not player_hitbox_disable.findIntersection( lava_pit_cmp ) )
      {
        // disable the rendering of this cell by removing its zorder component
        getReg().remove<Cmp::ZOrderValue>( lava_cell_entt );
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

void CryptSystem::checkSpikeTrapCollision()
{
  if ( Utils::get_player_mortality( getReg() ).state == Cmp::PlayerMortality::State::DEAD ) return;

  Cmp::Position player_pos = Utils::get_player_position( getReg() );
  Cmp::RectBounds player_hitbox( player_pos.position, player_pos.size, 0.5f );
  for ( auto [spike_trap_entt, spike_trap_cmp, spike_trap_anim_cmp] : getReg().view<Cmp::CryptPassageSpikeTrap, Cmp::SpriteAnimation>().each() )
  {
    if ( not spike_trap_anim_cmp.m_animation_active ) continue;

    Cmp::Position spike_trap_hitbox( spike_trap_cmp, Constants::kGridSquareSizePixelsF );
    if ( not player_hitbox.findIntersection( spike_trap_hitbox ) ) continue;
    Scene::CryptScene::stop_maze_timer();
    get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SKEWERED, spike_trap_hitbox ) );
  }
}

void CryptSystem::checkSpikeTrapActivationByProximity()
{
  auto player_pos_cmp = Utils::get_player_position( getReg() );
  Cmp::RectBounds player_hitbox_enable( player_pos_cmp.position, player_pos_cmp.size, 2.f );
  Cmp::RectBounds player_hitbox_disable( player_pos_cmp.position, player_pos_cmp.size, 5.f );
  for ( auto [spike_trap_entt, spike_trap_cmp, spike_trap_anim_cmp] : getReg().view<Cmp::CryptPassageSpikeTrap, Cmp::SpriteAnimation>().each() )
  {
    auto spike_trap_hitbox = sf::FloatRect( spike_trap_cmp, Constants::kGridSquareSizePixelsF );
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

std::vector<entt::entity> CryptSystem::getAvailableRoomPositions()
{
  // find all candidate positions in open rooms
  std::vector<entt::entity> internal_room_entts;
  for ( auto [pos_entt, candidate_pos_cmp] : getReg().view<Cmp::Position>().each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( Utils::get_player_position( getReg() ).findIntersection( open_room_cmp ) ) continue;
      if ( not candidate_pos_cmp.findIntersection( open_room_cmp ) ) continue;

      // search all Cmp::CryptRoomLavaPit for collision with positions in this open room
      bool intersects_lava = false;
      for ( auto [lava_pit_entt, lava_pit_cmp] : getReg().view<Cmp::CryptRoomLavaPit>().each() )
      {
        if ( candidate_pos_cmp.findIntersection( lava_pit_cmp ) )
        {
          intersects_lava = true;
          SPDLOG_DEBUG( "{},{} conflict with existing lava", candidate_pos_cmp.position.x, candidate_pos_cmp.position.y );
          break;
        }
      }

      // search all Cmp::CryptLever for collision with positions in this open room
      bool intersects_lever = false;
      for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : getReg().view<Cmp::CryptLever, Cmp::Position>().each() )
      {
        if ( candidate_pos_cmp.findIntersection( lever_pos_cmp ) )
        {
          intersects_lever = true;
          SPDLOG_DEBUG( "{},{} conflict with existing lever", candidate_pos_cmp.position.x, candidate_pos_cmp.position.y );
          break;
        }
      }

      // search all Cmp::CryptChest for collision with positions in this open room
      bool intersects_chest = false;
      for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : getReg().view<Cmp::CryptChest, Cmp::Position>().each() )
      {
        if ( candidate_pos_cmp.findIntersection( chest_pos_cmp ) )
        {
          intersects_chest = true;
          SPDLOG_DEBUG( "{},{} conflict with existing chest", candidate_pos_cmp.position.x, candidate_pos_cmp.position.y );
          break;
        }
      }

      // search all Cmp::CryptPassageBlock for collision with positions in this open room
      // Note: Cmp::CryptPassageBlock-owning entities do not own Cmp::Position components
      bool intersects_passageblock = false;
      for ( auto [pblock_entt, pblock_cmp] : getReg().view<Cmp::CryptPassageBlock>().each() )
      {
        Cmp::RectBounds expanded_candidate_pos_hitbox( candidate_pos_cmp.position, candidate_pos_cmp.size, 4.f );

        if ( expanded_candidate_pos_hitbox.findIntersection( sf::FloatRect( pblock_cmp, Constants::kGridSquareSizePixelsF ) ) )
        {
          intersects_passageblock = true;
          SPDLOG_DEBUG( "{},{} conflict with existing passageblock", candidate_pos_cmp.position.x, candidate_pos_cmp.position.y );
          break;
        }
      }

      if ( not intersects_lava and not intersects_lever and not intersects_chest and not intersects_passageblock )
      {
        internal_room_entts.push_back( pos_entt );
      }
    }
  }

  return internal_room_entts;
}

void CryptSystem::addChestToOpenRooms()
{
  auto internal_room_entts = getAvailableRoomPositions();
  Sprites::SpriteMetaType lever_sprite_type = "CRYPT.interior_chest";
  unsigned int disabled_lever_sprite_idx = 0;
  float zorder = m_sprite_factory.get_sprite_size_by_type( lever_sprite_type ).y;

  // add one lever to one room picked from the pool of candidates room positions
  Cmp::RandomInt room_position_picker( 0, internal_room_entts.size() - 1 );
  auto selected_entt = internal_room_entts[room_position_picker.gen()];
  auto room_pos = getReg().get<Cmp::Position>( selected_entt );
  Factory::CreateCryptChest( getReg(), room_pos.position, lever_sprite_type, disabled_lever_sprite_idx, zorder );
  SPDLOG_INFO( "Added chest to position: {},{}", room_pos.position.x, room_pos.position.y );
}

void CryptSystem::addLeverToOpenRooms()
{
  auto internal_room_entts = getAvailableRoomPositions();
  Sprites::SpriteMetaType lever_sprite_type = "CRYPT.interior_lever";
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

void CryptSystem::removeChestOpenRooms()
{
  for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : getReg().view<Cmp::CryptChest, Cmp::Position>().each() )
  {
    for ( auto [open_room_entt, open_room_cmp] : getReg().view<Cmp::CryptRoomOpen>().each() )
    {
      if ( not open_room_cmp.findIntersection( chest_pos_cmp ) ) continue;
      if ( open_room_cmp.findIntersection( Utils::get_player_position( getReg() ) ) ) continue;

      Factory::DestroyCryptLever( getReg(), chest_entt );
    }
  }
}

void CryptSystem::removeAllLevers()
{
  for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : getReg().view<Cmp::CryptChest, Cmp::Position>().each() )
  {
    Factory::DestroyCryptLever( getReg(), chest_entt );
  }
}

void CryptSystem::removeAllChests()
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
  const int npcs_to_spawn = std::min( Sys::PersistSystem::get<Cmp::Persist::CryptNpcSpawnCount>( getReg() ).get_value(),
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