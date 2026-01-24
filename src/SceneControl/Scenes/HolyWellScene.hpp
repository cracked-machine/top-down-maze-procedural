#ifndef SCENE_HOLYWELLSCENE_HPP_
#define SCENE_HOLYWELLSCENE_HPP_

#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Utils/Utils.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class SystemStore; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class HolyWellScene : public Scene<Events::ProcessHolyWellSceneInputEvent>
{
public:
  //! @brief The size of the crypt map grid in number of squares
  inline static constexpr sf::Vector2u kMapGridSize{ 15u, 15u };

  //! @brief The size of the crypt map grid in number of squares as floats
  inline static constexpr sf::Vector2f kMapGridSizeF{ static_cast<float>( kMapGridSize.x ), static_cast<float>( kMapGridSize.y ) };

  HolyWellScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher,
                 Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store ),
        m_sprite_Factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "HolyWellScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
  Sprites::SpriteFactory &m_sprite_Factory;
  Sprites::Containers::TileMap m_floormap{};

  sf::Vector2f m_player_start_position = Utils::snap_to_grid(
      sf::Vector2f( ( HolyWellScene::kMapGridSizeF.x / 2.f ) * Constants::kGridSquareSizePixels.x,
                    ( HolyWellScene::kMapGridSizeF.y - 3.f ) * Constants::kGridSquareSizePixels.y ) );
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_HOLYWELLSCENE_HPP_