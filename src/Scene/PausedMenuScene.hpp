#ifndef SCENE_PAUSEDMENUSCENE_HPP_
#define SCENE_PAUSEDMENUSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <SystemStore.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class PausedMenuScene : public IScene
{
public:
  PausedMenuScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "PausedMenuScene"; }

  entt::registry &get_registry() override;

  bool blocks_update() const override { return true; }

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_PAUSEDMENUSCENE_HPP_