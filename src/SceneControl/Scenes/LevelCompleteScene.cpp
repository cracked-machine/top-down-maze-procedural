#include <SceneControl/Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <SceneControl/Scenes/LevelCompleteScene.hpp>

namespace ProceduralMaze::Scene
{

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
}

void LevelCompleteScene::do_update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  render_menu_sys.render_victory_screen();
}

entt::registry &LevelCompleteScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene