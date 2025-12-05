#include <Events/ProcessGraveyardSceneInputEvent.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )

    : m_sound_bank( sound_bank ),
      m_system_store( system_store ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void GraveyardScene::on_init()
{
  // initialize any entities or components that should exist before on_enter
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &render_game_system = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();
  render_game_system.init_tilemap();

  // auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  // m_player_sys.addPlayerEntity();
  Factory::CreatePlayer( m_reg );

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate();

  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5 );

  auto &exit_sys = m_system_store.find<Sys::SystemStore::Type::ExitSystem>();
  exit_sys.spawn_exit();
  render_game_system.init_views();

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.init_hazard_field();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.init_hazard_field();

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.stopFootstepsSound();

  auto &m_render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  m_render_game_sys.clear_tilemap();
}

void GraveyardScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_music( "title_music" ).stop();
  // only do this once every update, other it constantly restarts the music
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Music::Status::Playing ) { m_sound_bank.get_music( "game_music" ).play(); }

  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );

  m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::BombSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().check_exit_collision();

  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();

  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );

  m_system_store.find<Sys::SystemStore::Type::WormholeSystem>().check_player_wormhole_collision();

  m_system_store.find<Sys::SystemStore::Type::DiggingSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );

  // clang-format off
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( 
    dt,
    m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>() 
  );
  // clang-format on

  // defer this scenes input event processing until we  exit this function
  m_nav_event_dispatcher.enqueue( Events::ProcessGraveyardSceneInputEvent() );
}

entt::registry &GraveyardScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene