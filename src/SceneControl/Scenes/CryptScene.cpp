#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>

namespace ProceduralMaze::Scene
{

void CryptScene::on_init()
{

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate( Sys::BaseSystem::kCryptMapGridSize, Sys::BaseSystem::kCryptMapGridOffset, false, false, false );

  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5 );

  Factory::FloormapFactory::CreateFloormap( m_floormap, Sys::BaseSystem::kCryptMapGridSize );
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  auto key_view = m_reg.view<Cmp::PlayerKeysCount>();
  for ( auto [entity, keycount] : key_view.each() )
  {
    SPDLOG_INFO( "KeyCount: {}", keycount.get_count() );
  }

  auto candle_view = m_reg.view<Cmp::PlayerCandlesCount>();
  for ( auto [entity, candlecount] : candle_view.each() )
  {
    SPDLOG_INFO( "CandleCount: {}", candlecount.get_count() );
  }

  auto relic_view = m_reg.view<Cmp::PlayerRelicCount>();
  for ( auto [entity, reliccount] : relic_view.each() )
  {
    SPDLOG_INFO( "RelicCount: {}", reliccount.get_count() );
  }
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
}

void CryptScene::do_update( [[maybe_unused]] sf::Time dt )
{

  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::DiggingSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::BombSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  // Note: this enqueues 'Events::SceneManagerEvent::Type::GAME_OVER' if player is dead
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );

  // clang-format off
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( 
    dt,
    m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>() , m_floormap
  );
  // clang-format on
}

entt::registry &CryptScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene