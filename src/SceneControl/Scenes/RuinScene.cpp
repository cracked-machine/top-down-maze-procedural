#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/System.hpp>
#include <SceneControl/Scenes/RuinScene.hpp>

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

void RuinScene::on_init()
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
  random_level_sys.gen_rectangle_gamearea( RuinScene::kMapGridSize, player_start_area, "RUIN.interior_wall" );

  // pass concrete spawn position to exit spawner
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().spawn_exit(
      sf::Vector2u{ RuinScene::kMapGridSize.x / 2, RuinScene::kMapGridSize.y - 1 } );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, RuinScene::kMapGridSize, "res/json/holywell_tilemap_config.json" );
}

void RuinScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  auto player_view = m_reg.view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( auto [player_entity, pc_cmp, pos_cmp] : player_view.each() )
  {
    pos_cmp.position = m_player_start_position;
  }
}

void RuinScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
}

void RuinScene::do_update( [[maybe_unused]] sf::Time dt )
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

entt::registry &RuinScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene