#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <SFML/System/Time.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Utils/Utils.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class SystemStore; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class GraveyardScene : public Scene<Events::ProcessGraveyardSceneInputEvent>
{
public:
  //! @brief The size of the graveyard map grid in number of squares
  inline static constexpr sf::Vector2u kMapGridSize{ 100u, 124u };

  //! @brief The size of the graveyard map grid in number of squares as floats
  inline static constexpr sf::Vector2f kMapGridSizeF{ static_cast<float>( GraveyardScene::kMapGridSize.x ),
                                                      static_cast<float>( GraveyardScene::kMapGridSize.y ) };

  GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
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
      GraveyardScene::kMapGridSize.x / 2.f * Constants::kGridSquareSizePixels.x,
      GraveyardScene::kMapGridSizeF.y / 2.f * Constants::kGridSquareSizePixels.y );

  sf::Clock m_scene_exit_cooldown{};
  sf::Time m_scene_exit_cooldown_time{ sf::seconds( 2 ) };
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_