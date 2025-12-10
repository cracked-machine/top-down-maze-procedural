#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class GraveyardScene : public Scene<Events::ProcessGraveyardSceneInputEvent>
{
public:
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store,
                  entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "GraveyardScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;

  Sprites::Containers::TileMap m_floormap{};

  inline static constexpr sf::Vector2f m_player_start_position = sf::Vector2f(
      Sys::BaseSystem::kGraveyardMapGridSizeF.x / 2.f * Sys::BaseSystem::kGridSquareSizePixels.x,
      Sys::BaseSystem::kGraveyardMapGridSizeF.y / 2.f * Sys::BaseSystem::kGridSquareSizePixels.y );
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_