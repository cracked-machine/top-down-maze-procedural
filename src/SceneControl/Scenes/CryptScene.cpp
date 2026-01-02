#include <SceneControl/Scenes/CryptScene.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>

namespace ProceduralMaze::Scene
{

void CryptScene::on_init()
{

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate( Sys::ProcGen::RandomLevelGenerator::AreaShape::CROSS, CryptScene::kMapGridSize,
                             Sys::ProcGen::RandomLevelGenerator::SceneType::CRYPT_INTERIOR );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, CryptScene::kMapGridSize, "res/json/crypt_tilemap_config.json" );

  // pass concrete spawn position to exit spawner
  m_system_store.find<Sys::SystemStore::Type::CryptSystem>().spawn_exit(
      sf::Vector2u{ CryptScene::kMapGridSize.x / 2, CryptScene::kMapGridSize.y - 1 } );
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
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

  if ( Utils::get_player_mortality( m_reg ).state != Cmp::PlayerMortality::State::DEAD )
  {
    m_system_store.find<Sys::SystemStore::Type::CryptSystem>().createRoomBorders();

    // make sure player has been situated in start room first
    m_system_store.find<Sys::SystemStore::Type::CryptSystem>().shuffle_rooms_passages();
    m_system_store.find<Sys::SystemStore::Type::CryptSystem>().reset_maze();
    get_maze_timer().restart();
  }
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
  get_maze_timer().reset();
}

void CryptScene::do_update( sf::Time dt )
{

  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::CryptSystem>().check_exit_collision();
  m_system_store.find<Sys::SystemStore::Type::CryptSystem>().update();

  // Note: this enqueues 'Events::SceneManagerEvent::Type::GAME_OVER' if player is dead
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::ON );
}

entt::registry &CryptScene::registry() { return m_reg; }

sf::Clock CryptScene::s_maze_timer;

} // namespace ProceduralMaze::Scene