#include <Components/Persistent/MusicVolume.hpp>
#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store )
{
}

void PausedMenuScene::on_init() { SPDLOG_INFO( "Initializing PausedMenuScene" ); }

void PausedMenuScene::on_enter()
{
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();
}

void PausedMenuScene::on_exit()
{
  // cleanup if needed
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.save_state();
}

void PausedMenuScene::update( [[maybe_unused]] sf::Time dt )
{
  auto &event_handler = m_system_store.find<Sys::SystemStore::Type::EventHandler>();
  event_handler.getEventDispatcher().trigger( Events::PauseClocksEvent() );
  // globalFrameClock.stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  while ( ( event_handler.paused_state_handler() != Sys::EventHandler::NavigationActions::RESUME ) )
  {
    // check for keyboard/window events to keep window responsive
    render_menu_sys.render_paused( dt );
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
  }
  // save persistent settings
  // event_handler.getEventDispatcher().trigger( Events::SaveSettingsEvent() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.save_state();

  // update music/sfx volumes with persistent settings
  auto &effects_volume = event_handler.get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = event_handler.get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  m_sound_bank.update_music_volume( music_volume );

  event_handler.getEventDispatcher().trigger( Events::ResumeClocksEvent() );
  // globalFrameClock.start();
  request( SceneRequest::PopOverlay );
}

entt::registry &PausedMenuScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene