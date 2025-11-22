#ifndef SCENE_GAMEOVERSCENE_HPP_
#define SCENE_GAMEOVERSCENE_HPP_

#include <SceneControl/IScene.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class GameOverScene : public IScene
{
public:
  GameOverScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "GameOverScene"; }

  entt::registry &get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;

  entt::dispatcher &m_nav_event_dispatcher;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GAMEOVERSCENE_HPP_