#include <Components/Persistent/EffectsVolume.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SceneControl/Scenes/PausedMenuScene.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <SceneControl/Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

void PausedMenuScene::on_init() { SPDLOG_INFO( "Initializing PausedMenuScene" ); }

void PausedMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering  {}", get_name() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();

  persistent_sys.get_systems_event_queue().trigger( Events::PauseClocksEvent() );
}

void PausedMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  persistent_sys.save_state();

  persistent_sys.get_systems_event_queue().trigger( Events::ResumeClocksEvent() );
}

void PausedMenuScene::do_update( [[maybe_unused]] sf::Time dt )
{

  // globalFrameClock.stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();

  // check for keyboard/window events to keep window responsive
  render_menu_sys.render_paused( dt );
  std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

  // save persistent settings
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  persistent_sys.save_state();

  // update music/sfx volumes with persistent settings
  auto &effects_volume = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::EffectsVolume>( m_reg ).get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MusicVolume>( m_reg ).get_value();
  m_sound_bank.update_music_volume( music_volume );

  // globalFrameClock.start();
}

entt::registry &PausedMenuScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene