#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/System.hpp>
#include <SceneControl/Scenes/HolyWellScene.hpp>

#include <Audio/SoundBank.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>

#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

void HolyWellScene::on_init()
{
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( HolyWellScene::kMapGridSize, player_start_area, "HOLYWELL.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // pass concrete spawn position to exit spawner
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().spawn_exit(
      sf::Vector2u{ HolyWellScene::kMapGridSize.x / 2, HolyWellScene::kMapGridSize.y - 1 } );

  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().spawn_well( sf::Vector2u{ ( HolyWellScene::kMapGridSize.x / 2 ) - 1, 4 } );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, HolyWellScene::kMapGridSize, "res/json/holywell_tilemap_config.json" );

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void HolyWellScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  auto player_view = m_reg.view<Cmp::PlayerCharacter, Cmp::Position>();
  for ( auto [player_entity, pc_cmp, pos_cmp] : player_view.each() )
  {
    pos_cmp.position = m_player_start_position;
  }
}

void HolyWellScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void HolyWellScene::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().check_exit_collision();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::OFF );
}

entt::registry &HolyWellScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene