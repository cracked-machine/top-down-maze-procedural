#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Scene/SettingsMenuScene.hpp>
#include <Scene/TitleScene.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

TitleScene::TitleScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store )
{
}

void TitleScene::on_init() { SPDLOG_INFO( "Initializing TitleScene" ); }

void TitleScene::on_enter()
{
  SPDLOG_INFO( "Entering TitleScene" );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto &m_render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  m_render_menu_sys.init_title();

  // update fx volumes with persistent settings
  auto &event_handler = m_system_store.find<Sys::SystemStore::Type::EventHandler>();
  auto &effects_volume = event_handler.get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto &music_volume = event_handler.get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
  m_sound_bank.update_music_volume( music_volume );

  if ( m_sound_bank.get_music( "title_music" ).getStatus() != sf::Music::Status::Playing )
  {
    m_sound_bank.get_music( "title_music" ).play();
  }
}

void TitleScene::on_exit()
{
  // Cleanup if needed
  SPDLOG_INFO( "Exiting TitleScene" );

  //   m_persistent_sys->save_state();
}

void TitleScene::update( [[maybe_unused]] sf::Time dt )
{
  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  render_menu_sys.render_title();

  auto &event_handler = m_system_store.find<Sys::SystemStore::Type::EventHandler>();
  auto menu_action = event_handler.menu_state_handler();
  switch ( menu_action )
  {
    case Sys::EventHandler::NavigationActions::PLAY:
      request( SceneRequest::GraveyardScene );
      break;
    case Sys::EventHandler::NavigationActions::SETTINGS:
      request( SceneRequest::SettingsMenu );
      break;
    case Sys::EventHandler::NavigationActions::EXIT:
      request( SceneRequest::Quit );
      break;
    default:
      break;
  }
}

entt::registry *TitleScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene