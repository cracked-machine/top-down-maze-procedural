#include <Components/Persistent/MusicVolume.hpp>
#include <Events/ProcessGraveyardSceneInputEvent.hpp>
#include <Events/ProcessPausedMenuSceneInputEvent copy.hpp>
#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                                  Sys::EventHandler *event_handler, Sys::RenderMenuSystem *render_menu_sys,
                                  entt::dispatcher &nav_event_dispatcher )
    : m_sound_bank( sound_bank ),
      m_persistent_sys( persistent_sys ),
      m_event_handler( event_handler ),
      m_render_menu_sys( render_menu_sys ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void PausedMenuScene::on_init() { SPDLOG_INFO( "Initializing PausedMenuScene" ); }

void PausedMenuScene::on_enter()
{
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
  m_event_handler->getEventDispatcher().trigger( Events::PauseClocksEvent() );
}

void PausedMenuScene::on_exit()
{
  // cleanup if needed

  m_event_handler->getEventDispatcher().trigger( Events::ResumeClocksEvent() );
  m_persistent_sys->save_state();
}

void PausedMenuScene::update( [[maybe_unused]] sf::Time dt )
{
  m_nav_event_dispatcher.trigger<Events::ProcessPausedMenuSceneInputEvent>();
  m_render_menu_sys->render_paused( dt );

  // auto &effects_volume = m_event_handler->get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  // m_sound_bank.update_effects_volume( effects_volume );
  // auto &music_volume = m_event_handler->get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  // m_sound_bank.update_music_volume( music_volume );

  std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
}

entt::registry *PausedMenuScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene