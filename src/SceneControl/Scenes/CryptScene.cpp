#include <Audio/SoundBank.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/SceneData.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/PassageSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Player.hpp>

namespace ProceduralMaze::Scene
{

void CryptScene::on_init()
{

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_scene_map_data = std::make_shared<SceneData>( "res/scenes/crypt.json" );
  SPDLOG_INFO( "get_floor_image: {}", m_scene_map_data->floor_tileset_image().string() );
  SPDLOG_INFO( "levelgen_tilelayer size: {}", m_scene_map_data->levelgen_tilelayer().size() );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene configuration (json) data
  auto [_, player_start_pos_px] = m_scene_map_data->get_player_start_position();
  SPDLOG_INFO( "player start position {},{}", player_start_pos_px.x, player_start_pos_px.y );
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );

  auto [map_size_grid, map_size_pixel] = m_scene_map_data->map_size();

  // create the empty game area
  auto player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_position, Constants::kGridSizePxF, 3.f, Cmp::RectBounds::ScaleAxis::XY );
  auto &random_level_sys = m_sys.find<Sys::Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_game_area( *m_scene_map_data );
  m_sys.find<Sys::Store::Type::PassageSystem>().init_scene_data( m_scene_map_data );

  // intialise the game area
  auto start_room_entity = m_reg.create();
  m_reg.emplace_or_replace<Cmp::CryptRoomStart>( start_room_entity, player_start_area.position(), player_start_area.size() );
  m_sys.find<Sys::Store::Type::CryptSystem>().create_end_room( map_size_grid );
  m_sys.find<Sys::Store::Type::CryptSystem>().create_initial_crypt_rooms( map_size_grid );
  m_sys.find<Sys::Store::Type::CryptSystem>().cache_all_room_connections();
  m_sys.find<Sys::Store::Type::CryptSystem>().gen_crypt_initial_interior();

  // create a navmesh for pathfinding in the scene
  m_pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    m_pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PassageSystem>().init_nav_mesh( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::CryptSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );

  m_floormap.create( random_level_sys.get_void_sm(), m_scene_map_data );
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

  auto &player_pos = Utils::Player::get_position( m_reg );
  player_pos.position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );

  m_sys.find<Sys::Store::Type::CryptSystem>().setup();
  get_maze_timer().restart();
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
  get_maze_timer().reset();

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void CryptScene::do_update( sf::Time dt )
{

  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::CryptSystem>().update();
  m_sys.find<Sys::Store::Type::ShockwaveSystem>().checkShockwavePlayerCollision();
  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );
  m_sys.find<Sys::Store::Type::PassageSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::DarkMode::ON );
}

entt::registry &CryptScene::registry() { return m_reg; }

sf::Clock CryptScene::s_maze_timer;

} // namespace ProceduralMaze::Scene