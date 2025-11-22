#ifndef SCENE_LEVELCOMPLETE_SCENE_HPP_
#define SCENE_LEVELCOMPLETE_SCENE_HPP_

#include <Audio/SoundBank.hpp>
#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <SystemStore.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class LevelCompleteScene : public IScene
{
public:
  LevelCompleteScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "LevelCompleteScene"; }

  entt::registry &get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;

  entt::dispatcher &m_nav_event_dispatcher;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_LEVELCOMPLETE_SCENE_HPP_