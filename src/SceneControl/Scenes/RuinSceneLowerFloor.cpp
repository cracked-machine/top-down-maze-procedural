#include <Audio/SoundBank.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scenes/RuinSceneLowerFloor.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneLowerFloor::on_init()
{
  auto gridsize = Constants::kGridSizePxF;
  using SystemStoreType = Sys::Store::Type;

  auto &m_persistent_sys = m_sys.find<SystemStoreType::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  m_scene_map_data = std::make_shared<SceneData>( "res/scenes/ruinlower.json" );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene data
  auto [_, player_start_pos_px] = m_scene_map_data->get_player_start_position();
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );

  auto [map_size_grid, map_size_pixel] = m_scene_map_data->map_size();

  // generate the empty game area
  sf::Vector2f player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_position, gridsize, 1.f, Cmp::RectBounds::ScaleAxis::XY );
  auto &random_level_sys = m_sys.find<SystemStoreType::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_game_area( *m_scene_map_data );

  // add access hitbox just above horizontal centerpoint
  sf::Vector2f flooraccess_position( map_size_pixel.x - ( 3 * gridsize.x ), 2 * gridsize.y );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_sys.find<SystemStoreType::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size, Cmp::RuinFloorAccess::Direction::TO_UPPER );

  m_floormap.create( random_level_sys.get_void_sm(), m_scene_map_data );

  sf::Vector2f bc_area_position( 0, 0 );
  sf::Vector2f bc_area_size( map_size_pixel.x - 48, map_size_pixel.y - 16 );
  m_sys.find<SystemStoreType::RuinSystem>().gen_lowerfloor_bookcases( sf::FloatRect( bc_area_position, bc_area_size ) );

  sf::Vector2f cobweb_area_position( 0, 0 );
  sf::Vector2f cobweb_area_size( map_size_pixel.x - 48, map_size_pixel.y - 32 );
  m_sys.find<SystemStoreType::RuinSystem>().add_lowerfloor_cobwebs( 200, sf::FloatRect( cobweb_area_position, cobweb_area_size ) );

  m_sys.find<Sys::Store::Type::RuinSystem>().reset_player_curse();

  // create a navmesh for pathfinding in the scene
  m_pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    m_pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "game_music" ).stop();

  auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( m_reg );
  if ( inventory_type != "sprite.item.witchesjar" )
  {
    if ( m_sound_bank.get_music( "ruin_creaking_rope" ).getStatus() != sf::Sound::Status::Playing )
    {
      m_sound_bank.get_music( "ruin_creaking_rope" ).play();
      m_sound_bank.get_music( "ruin_creaking_rope" ).setLooping( true );
    }
    if ( m_sound_bank.get_music( "ruin_music" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_music( "ruin_music" ).play(); }
  }

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_world_view();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::Player::remove_lerp_cmp( m_reg );

  // prevent the player from wandering off before the scene has loaded
  auto &player_dir = Utils::Player::get_direction( m_reg );
  player_dir = Cmp::Direction{ { 0.f, 0.f } };

  auto &player_pos = Utils::Player::get_position( m_reg );
  switch ( m_entry_mode )
  {
    case EntryMode::FROM_DOOR: {
      SPDLOG_INFO( "Player entering from door" );
      player_pos.position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
      break;
    }
    case EntryMode::FROM_UPPER_FLOOR: {
      SPDLOG_INFO( "Player entering from upper floor" );
      player_pos.position = Utils::snap_to_grid( player_pos.position );
      break;
    }
  }
  SPDLOG_INFO( "Player entered RuinSceneLowerFloor at position ({}, {})", player_pos.position.x, player_pos.position.y );

  auto player_entt = Utils::Player::get_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::LOWER );

  m_sys.find<Sys::Store::Type::RuinSystem>().reset_floor_access_cooldown();

  // re-initialise the spatial grid weak_ptr for when we return from upper floor scene
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );
}

void RuinSceneLowerFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_sound_bank.get_music( "ruin_creaking_rope" ).stop();
  m_sound_bank.get_music( "ruin_music" ).stop();

  m_reg.clear();

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  using namespace Sys;
  m_sys.find<Store::Type::AnimSystem>().update( dt );
  m_sys.find<Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Store::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_UPPER );
  m_sys.find<Store::Type::RuinSystem>().check_movement_slowdowns();
  m_sys.find<Store::Type::RuinSystem>().creaking_rope_update();

  m_sys.find<Store::Type::PlayerSystem>().update( dt, Sys::PlayerSystem::FootStepSfx::NONE );
  m_sys.find<Store::Type::PlayerSystem>().disable_damage_cooldown();

  auto [_, map_size_pixel] = m_scene_map_data->map_size();

  bool is_player_cursed = m_sys.find<Sys::Store::Type::RuinSystem>().check_activate_player_curse( map_size_pixel );
  if ( is_player_cursed ) { m_sys.find<Store::Type::RuinSystem>().check_create_witch( m_reg, sf::FloatRect( { 0, 0 }, map_size_pixel ) ); }

  // `check_exit_collision()` may reset the player curse so it must be called after `check_activate_player_curse()`
  // or we incorrectly re-trigger the curse effects before we can leave this function and exit the scene.
  // i.e. the witch scream triggers again as we leave the scene
  m_sys.find<Store::Type::RuinSystem>().check_exit_collision();

  auto &overlay_sys = m_sys.find<Store::Type::RenderOverlaySystem>();
  m_sys.find<Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, DarkMode::OFF, WeatherMode::OFF,
                                                           ( is_player_cursed ? CursedMode::ON : CursedMode::OFF ) );
}

entt::registry &RuinSceneLowerFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene