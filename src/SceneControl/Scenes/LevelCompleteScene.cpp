#include <Player.hpp>
#include <Player/PlayerCadaverCount.hpp>
#include <Player/PlayerWealth.hpp>
#include <SFML/System/Time.hpp>
#include <SceneControl/Scenes/LevelCompleteScene.hpp>

#include <Audio/SoundBank.hpp>
#include <SceneControl/Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

void LevelCompleteScene::on_init() { /* init entities */ }
void LevelCompleteScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  auto &persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  persistent_sys.initialize_component_registry();
  persistent_sys.load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}

void LevelCompleteScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  auto &player_sys = m_sys.find<Sys::Store::Type::PlayerSystem>();
  player_sys.stopFootstepsSound();
}

void LevelCompleteScene::do_update( sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &wealth = Utils::Player::get_wealth( m_reg );
  auto &cadaver_count = Utils::Player::get_cadaver_count( m_reg );

  static constexpr float kPlantCheckIntervalHz = 1.0f;
  m_scorecheck_accumulator += dt;
  if ( m_scorecheck_accumulator.asSeconds() >= 1.f / kPlantCheckIntervalHz )
  {
    if ( cadaver_count.get_count() > 0 )
    {
      cadaver_count.decrement_count( 1 );
      wealth.wealth += 10;
      m_sound_bank.get_effect( "get_loot" ).play();
    }
    m_scorecheck_accumulator = sf::Time::Zero;
  }

  auto &render_menu_sys = m_sys.find<Sys::Store::Type::RenderMenuSystem>();
  render_menu_sys.render_victory_screen( ( cadaver_count.get_count() > 0 ? false : true ) );
}

entt::registry &LevelCompleteScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene