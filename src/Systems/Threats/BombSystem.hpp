#ifndef SRC_SYSTEMS_BOMBSYSTEM_HPP__
#define SRC_SYSTEMS_BOMBSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Utils/Utils.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Events/PlayerActionEvent.hpp>

// clang-format off
namespace ProceduralMaze::Sprites { class SpriteFactory; }
namespace ProceduralMaze::Sys { class SystemStore; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Sys
{

// this currently only supports one bomb at a time
class BombSystem : public BaseSystem
{
public:
  BombSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override;
  //! @brief event handlers for resuming system clocks
  void onResume() override;

  void arm_grave_bomb();
  void arm_player_bomb();
  void place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int blast_radius );
  void update();

  /// EVENTS
  void on_bomb_event( const Events::PlayerActionEvent &event );

private:
  const sf::Vector2f max_explosion_zone_size{ Constants::kGridSquareSizePixels.x * 3.f, Constants::kGridSquareSizePixels.y * 3.f };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_BOMBSYSTEM_HPP__