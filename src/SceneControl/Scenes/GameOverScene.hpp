#ifndef SRC_SCENECONTROL_SCENES_GAMEOVERSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_GAMEOVERSCENE_HPP__

#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

//! @brief The game-over screen shown when the player dies: stops gameplay audio and renders the defeat screen.
class GameOverScene : public Scene<Events::ProcessGameoverSceneInputEvent>
{
public:
  //! @brief Construct a new GameOverScene object
  //! @param sound_bank Shared sound bank used to play/stop audio for this scene
  //! @param system_store Store of game systems the scene drives each update
  //! @param nav_event_dispatcher Used to notify the SceneInputRouter when new input events are available
  GameOverScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store )
  {
  }

  //! @brief Logs scene initialization; no world entities are created for this scene
  void on_init() override;

  //! @brief Reloads persistent state and stops the "bubbling_lava" sound effect
  void on_enter() override;

  //! @brief Clears the registry and stops footstep audio
  void on_exit() override;

  //! @brief Get the name of the scene
  //! @return std::string "GameOverScene"
  std::string get_name() const override { return "GameOverScene"; }

  //! @brief Get the registry object owned by the scene
  //! @return entt::registry&
  entt::registry &registry() override;

protected:
  //! @brief Stops the footsteps sound effect and renders the defeat screen
  //! @param dt The time elapsed since the last update
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  //! @brief Shared sound bank used to play/stop audio for this scene
  Audio::SoundBank &m_sound_bank;
  //! @brief Store of game systems the scene drives each update
  Sys::Store &m_sys;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_GAMEOVERSCENE_HPP__
