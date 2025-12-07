
#include <Components/Persistent/MusicVolume.hpp>
#include <Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <SceneControl/Scenes/PausedMenuScene.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void PausedMenuScene::on_init() { SPDLOG_INFO( "Initializing PausedMenuScene" ); }

void PausedMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering  {}", get_name() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();

  persistent_sys.get_systems_event_queue().trigger( Events::PauseClocksEvent() );
}

void PausedMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.save_state();

  persistent_sys.get_systems_event_queue().trigger( Events::ResumeClocksEvent() );
}

void PausedMenuScene::update( [[maybe_unused]] sf::Time dt )
{

  // globalFrameClock.stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();

  // check for keyboard/window events to keep window responsive
  render_menu_sys.render_paused( dt );
  std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

  // Notify SceneInputRouter that there may be new PausedMenuScene input to process
  m_nav_event_dispatcher.enqueue( Events::ProcessPausedMenuSceneInputEvent() );

  // save persistent settings
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.save_state();

  // update music/sfx volumes with persistent settings
  auto &effects_volume = persistent_sys.get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = persistent_sys.get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  m_sound_bank.update_music_volume( music_volume );

  // globalFrameClock.start();
}

entt::registry &PausedMenuScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene