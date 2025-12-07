#include <Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <SceneControl/Scenes/LevelCompleteScene.hpp>

namespace ProceduralMaze::Scene
{

LevelCompleteScene::LevelCompleteScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void LevelCompleteScene::on_init() { /* init entities */ }
void LevelCompleteScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  auto &persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}

void LevelCompleteScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  auto &player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  player_sys.stopFootstepsSound();

  auto &m_render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  m_render_game_sys.clear_tilemap();
}

void LevelCompleteScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  render_menu_sys.render_victory_screen();

  // Notify SceneInputRouter that there may be new LevelCompleteScene input to process
  m_nav_event_dispatcher.enqueue( Events::ProcessLevelCompleteSceneInputEvent() );
}

entt::registry &LevelCompleteScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene