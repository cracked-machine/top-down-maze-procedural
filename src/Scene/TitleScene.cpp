#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Events/ProcessTitleSceneInputEvent.hpp>
#include <Scene/SettingsMenuScene.hpp>
#include <Scene/TitleScene.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

TitleScene::TitleScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                        Sys::RenderMenuSystem *render_menu_sys, entt::dispatcher &nav_event_dispatcher )
    : m_sound_bank( sound_bank ),
      m_persistent_sys( persistent_sys ),
      m_render_menu_sys( render_menu_sys ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void TitleScene::on_init() { SPDLOG_INFO( "Initializing {}", get_name() ); }

void TitleScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();

  m_render_menu_sys->init_title();

  // update fx volumes with persistent settings
  auto &effects_volume = m_persistent_sys->get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = m_persistent_sys->get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  m_sound_bank.update_music_volume( music_volume );

  if ( m_sound_bank.get_music( "title_music" ).getStatus() != sf::Music::Status::Playing )
  {
    m_sound_bank.get_music( "title_music" ).play();
  }
}

void TitleScene::on_exit()
{
  // Cleanup if needed
  SPDLOG_INFO( "Exiting {}", get_name() );

  //   m_persistent_sys->save_state();
}

void TitleScene::update( [[maybe_unused]] sf::Time dt )
{
  m_render_menu_sys->render_title();
  m_nav_event_dispatcher.trigger<Events::ProcessTitleSceneInputEvent>();
}

entt::registry *TitleScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene