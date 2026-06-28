#ifndef SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP__

#include <Components/Shop/ShopInventory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <SceneControl/Events/ProcessShopSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/VertexFloor.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

class ShopScene : public Scene<Events::ProcessShopSceneInputEvent>
{
public:
  ShopScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher, Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_factory( sprite_Factory )
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
  void reinit_navmesh();

  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;
  Sprites::Containers::VertexFloor m_floormap;

  bool m_overlay_open{ false };
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_SHOPSCENE_HPP__
