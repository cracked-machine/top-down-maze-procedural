#ifndef SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_
#define SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_

#include <Audio/SoundBank.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessRuinSceneLowerInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/SystemStore.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Scene
{

class RuinSceneLowerFloor : public Scene<Events::ProcessRuinSceneLowerInputEvent>
{
public:
  //! @brief The size of the crypt map grid in number of squares
  inline static constexpr sf::Vector2u kMapGridSize{ 9u, 5u };

  //! @brief The size of the crypt map grid in number of squares as floats
  inline static constexpr sf::Vector2f kMapGridSizeF{ static_cast<float>( kMapGridSize.x * Constants::kGridSquareSizePixelsF.x ),
                                                      static_cast<float>( kMapGridSize.y *Constants::kGridSquareSizePixelsF.y ) };

  RuinSceneLowerFloor( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher,
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
  std::string get_name() const override { return "RuinSceneLowerFloor"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
  Sprites::SpriteFactory &m_sprite_Factory;
  Sprites::Containers::TileMap m_floormap{};

  sf::Vector2f m_player_start_position = Utils::snap_to_grid(
      sf::Vector2f( ( RuinSceneLowerFloor::kMapGridSizeF.x / 2.f ) - Constants::kGridSquareSizePixels.x,
                    ( RuinSceneLowerFloor::kMapGridSizeF.y - ( 2.f * Constants::kGridSquareSizePixels.y ) ) ) );
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_