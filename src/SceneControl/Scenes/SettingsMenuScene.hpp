#ifndef SRC_SCENECONTROL_SCENES_SETTINGSMENUSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_SETTINGSMENUSCENE_HPP__

#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

//! @brief The settings menu screen: lets the player adjust and persist game settings (e.g. volume).
class SettingsMenuScene : public Scene<Events::ProcessSettingsMenuSceneInputEvent>
{
public:
  //! @brief Construct a new SettingsMenuScene object
  //! @param system_store Store of game systems the scene drives each update
  //! @param nav_event_dispatcher Used to notify the SceneInputRouter when new input events are available
  SettingsMenuScene( Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sys( system_store )
  {
  }

  //! @brief Logs scene initialization; no world entities are created for this scene
  void on_init() override;

  //! @brief Reloads persistent state so the current settings are shown
  void on_enter() override;

  //! @brief Saves persistent state so any changed settings are kept
  void on_exit() override;

  //! @brief Get the name of the scene
  //! @return std::string "SettingsMenuScene"
  [[nodiscard]] std::string get_name() const override { return "SettingsMenuScene"; }

  //! @brief Get the registry object owned by the scene
  //! @return entt::registry&
  entt::registry &registry() override;

protected:
  //! @brief Renders the settings menu
  //! @param dt The time elapsed since the last update
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  //! @brief Store of game systems the scene drives each update
  Sys::Store &m_sys;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_SETTINGSMENUSCENE_HPP__
