#include <SceneControl/Scenes/GameOverScene.hpp>

#include <Audio/SoundBank.hpp>
#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

void GameOverScene::on_init() { SPDLOG_INFO( "Initializing GameOverScene" ); }
void GameOverScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  auto &persistent_sys = static_cast<Sys::PersistSystem &>( m_sys.find<Sys::Store::Type::PersistSystem>() );
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}
void GameOverScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  auto &m_player_sys = m_sys.find<Sys::Store::Type::PlayerSystem>();
  m_player_sys.stopFootstepsSound();
}

void GameOverScene::do_update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &render_menu_sys = m_sys.find<Sys::Store::Type::RenderMenuSystem>();
  render_menu_sys.render_defeat_screen();
}

entt::registry &GameOverScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene