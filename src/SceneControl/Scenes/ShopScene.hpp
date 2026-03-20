#ifndef SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP_
#define SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP_

#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessShopSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Shop/ShopInventory.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Utils/Utils.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class ShopScene : public Scene<Events::ProcessShopSceneInputEvent>
{
public:
  ShopScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher, Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_Factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "ShopScene"; }

  entt::registry &registry() override;

  void open_overlay();
  void close_overlay();
  bool is_overlay_open() const { return m_overlay_open; }

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_Factory;
  Sprites::Containers::TileMap m_floormap{};

  Cmp::ShopInventory::Config m_inventory_config;
  bool m_overlay_open{ false };
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP_