#ifndef SCENE_CRYPTSCENE_HPP_
#define SCENE_CRYPTSCENE_HPP_

#include <SceneControl/Events/ProcessCryptSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class CryptScene : public Scene<Events::ProcessCryptSceneInputEvent>
{
public:
  //! @brief The size of the crypt map grid in number of squares
  inline static constexpr sf::Vector2u kMapGridSize{ 64u, 32u };

  //! @brief The size of the crypt map grid in number of squares as floats
  inline static constexpr sf::Vector2f kMapGridSizeF{ static_cast<float>( kMapGridSize.x ),
                                                      static_cast<float>( kMapGridSize.y ) };

  CryptScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "CryptScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
  Sprites::Containers::TileMap m_floormap{};

  inline static constexpr sf::Vector2f m_player_start_position = sf::Vector2f(
      CryptScene::kMapGridSizeF.x / 2.f * Constants::kGridSquareSizePixels.x,
      CryptScene::kMapGridSizeF.y / 2.f * Constants::kGridSquareSizePixels.y );
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_CRYPTSCENE_HPP_