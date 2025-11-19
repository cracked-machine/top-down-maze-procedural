#include <Components/Persistent/MusicVolume.hpp>
#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                                  Sys::EventHandler *event_handler, Sys::RenderMenuSystem *render_menu_sys )
    : m_sound_bank( sound_bank ),
      m_persistent_sys( persistent_sys ),
      m_event_handler( event_handler ),
      m_render_menu_sys( render_menu_sys )
{
}

void PausedMenuScene::on_init() { SPDLOG_INFO( "Initializing PausedMenuScene" ); }

void PausedMenuScene::on_enter()
{
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
}

void PausedMenuScene::on_exit()
{
  // cleanup if needed
  m_persistent_sys->save_state();
}

void PausedMenuScene::update( [[maybe_unused]] sf::Time dt )
{

  m_event_handler->getEventDispatcher().trigger( Events::PauseClocksEvent() );
  // globalFrameClock.stop();

  while ( ( m_event_handler->paused_state_handler() != Sys::EventHandler::MenuAction::PLAY ) )
  {
    // check for keyboard/window events to keep window responsive
    m_render_menu_sys->render_paused( dt );
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
  }
  // save persistent settings
  // m_event_handler->getEventDispatcher().trigger( Events::SaveSettingsEvent() );
  m_persistent_sys->save_state();

  // update music/sfx volumes with persistent settings
  auto &effects_volume = m_event_handler->get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = m_event_handler->get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  m_sound_bank.update_music_volume( music_volume );

  m_event_handler->getEventDispatcher().trigger( Events::ResumeClocksEvent() );
  // globalFrameClock.start();
  request( SceneRequest::Resume );
}

entt::registry *PausedMenuScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene