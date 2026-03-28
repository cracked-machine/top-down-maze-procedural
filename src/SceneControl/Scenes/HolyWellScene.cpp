#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/System.hpp>
#include <Constants.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <Player.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scenes/HolyWellScene.hpp>
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

namespace ProceduralMaze::Scene
{

void HolyWellScene::on_init()
{
  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_scene_map_data = std::make_shared<SceneData>( "res/scenes/well.json" );
  SPDLOG_INFO( "wall_tilelayer size: {}", m_scene_map_data->wall_tilelayer().size() );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene configuration (json) data
  auto [_, player_start_pos_px] = m_scene_map_data->get_player_start_position();
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );

  // create the empty game area
  sf::Vector2f player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_position, Constants::kGridSizePxF, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );
  auto &random_level_sys = m_sys.find<Sys::Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_game_area( *m_scene_map_data );

  m_floormap.create( random_level_sys.get_void_sm(), m_scene_map_data );

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

void HolyWellScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  m_sound_bank.get_music( "game_music" ).stop();

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

  auto &player_pos = Utils::Player::get_position( m_reg );
  player_pos.position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
}

void HolyWellScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void HolyWellScene::do_update( sf::Time dt )
{
  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::HolyWellSystem>().check_exit_collision();
  m_sys.find<Sys::Store::Type::HolyWellSystem>().check_inventory_deposit( dt );

  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::DarkMode::OFF, Sys::WeatherMode::OFF );
}

entt::registry &HolyWellScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene