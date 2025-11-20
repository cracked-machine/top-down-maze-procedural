#ifndef SCENE_GAMEOVERSCENE_HPP_
#define SCENE_GAMEOVERSCENE_HPP_

#include <Audio/SoundBank.hpp>
#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <SystemStore.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class GameOverScene : public IScene
{
public:
  GameOverScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "GameOverScene"; }

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GAMEOVERSCENE_HPP_