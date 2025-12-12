#include <Components/Persistent/MusicVolume.hpp>
#include <SceneControl/Events/ProcessTitleSceneInputEvent.hpp>
#include <SceneControl/Scenes/TitleScene.hpp>

namespace ProceduralMaze::Scene
{

void TitleScene::on_init() { SPDLOG_INFO( "Initializing TitleScene" ); }

void TitleScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto &m_render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  m_render_menu_sys.init_title();

  // update fx volumes with persistent settings
  auto &effects_volume = Sys::PersistentSystem::get_persistent_component<
                             Cmp::Persistent::EffectsVolume>( m_reg )
                             .get_value();
  m_sound_bank.update_effects_volume( effects_volume );
  auto
      &music_volume = Sys::PersistentSystem::get_persistent_component<Cmp::Persistent::MusicVolume>(
                          m_reg )
                          .get_value();
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
}

void TitleScene::do_update( [[maybe_unused]] sf::Time dt )
{
  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  render_menu_sys.render_title();
}

entt::registry &TitleScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene