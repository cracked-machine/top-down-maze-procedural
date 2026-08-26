#ifndef SRC_SCENECONTROL_SCENES_TITLESCENE_HPP__
#define SRC_SCENECONTROL_SCENES_TITLESCENE_HPP__

#include <SceneControl/Events/ProcessTitleSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

//! @brief The title screen: the game's entry-point scene, applying the title shader and playing title music.
class TitleScene : public Scene<Events::ProcessTitleSceneInputEvent>
{
public:
  //! @brief Construct a new TitleScene object
  //! @param sound_bank Shared sound bank used to play/stop title-screen audio
  //! @param system_store Store of game systems the scene drives each update
  //! @param nav_event_dispatcher Used to notify the SceneInputRouter when new input events are available
  TitleScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store )
  {
  }

  //! @brief Reloads persistent state and adds the title screen shader
  void on_init() override;

  //! @brief Reloads persistent state, applies persisted music/effects volume, and starts looping title music
  void on_enter() override;

  //! @brief Logs scene exit; no cleanup is currently required
  void on_exit() override;

  //! @brief Get the name of the scene
  //! @return std::string "TitleScene"
  std::string get_name() const override { return "TitleScene"; }

  //! @brief Get the registry object owned by the scene
  //! @return entt::registry&
  entt::registry &registry() override;

protected:
  //! @brief Renders the title screen
  //! @param dt The time elapsed since the last update
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  //! @brief Shared sound bank used to play/stop title-screen audio
  Audio::SoundBank &m_sound_bank;

  //! @brief Store of game systems the scene drives each update
  Sys::Store &m_sys;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_TITLESCENE_HPP__
