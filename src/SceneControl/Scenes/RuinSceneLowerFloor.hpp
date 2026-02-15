#ifndef SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_
#define SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_

#include <Audio/SoundBank.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessRuinSceneLowerInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/SystemStore.hpp>
#include <Utils/Utils.hpp>

#include <Sprites/SpriteMetaType.hpp>
namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Scene
{

class RuinSceneLowerFloor : public Scene<Events::ProcessRuinSceneLowerInputEvent>
{
public:
  enum class EntryMode { FROM_DOOR, FROM_UPPER_FLOOR };

  //! @brief The size of the crypt map grid in number of squares
  inline static constexpr sf::Vector2u kMapGridSize{ 19u, 10u };

  //! @brief The size of the crypt map grid in number of squares as floats
  inline static constexpr sf::Vector2f kMapGridSizeF{ static_cast<float>( kMapGridSize.x * Constants::kGridSquareSizePixelsF.x ),
                                                      static_cast<float>( kMapGridSize.y *Constants::kGridSquareSizePixelsF.y ) };

  RuinSceneLowerFloor( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher,
                       Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store ),
        m_sprite_factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "RuinSceneLowerFloor"; }

  entt::registry &registry() override;

  void set_entry_mode( EntryMode entry_mode ) { m_entry_mode = entry_mode; }

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
  Sprites::SpriteFactory &m_sprite_factory;
  Sprites::Containers::TileMap m_floormap{};

  EntryMode m_entry_mode;
  //! @brief Player position after entering through the front door
  sf::Vector2f m_player_door_position = Utils::snap_to_grid(
      sf::Vector2f( ( RuinSceneLowerFloor::kMapGridSizeF.x / 3.f ) - Constants::kGridSquareSizePixels.x,
                    ( RuinSceneLowerFloor::kMapGridSizeF.y - ( 2.f * Constants::kGridSquareSizePixels.y ) ) ) );

  //! @brief Player position after entering from upper floor
  sf::Vector2f m_player_stair_position = Utils::snap_to_grid(
      { RuinSceneLowerFloor::kMapGridSizeF.x - ( 2 * Constants::kGridSquareSizePixelsF.x ),
        RuinSceneLowerFloor::kMapGridSizeF.y - ( 2 * Constants::kGridSquareSizePixelsF.y ) } );
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_