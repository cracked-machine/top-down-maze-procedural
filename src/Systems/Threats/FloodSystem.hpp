#ifndef __SYSTEMS_WATER_SYSTEM_HPP__
#define __SYSTEMS_WATER_SYSTEM_HPP__

#include <entt/entity/registry.hpp>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/Persistent/FloodSpeed.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <Systems/BaseSystem.hpp>

#include <unordered_map>

namespace ProceduralMaze::Sys {

class FloodSystem : public BaseSystem
{
public:
  FloodSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory );

  ~FloodSystem() = default;

  void add_flood_water_entity();

  void update();

private:
  void updateFlood( float dt );

private:
  static constexpr float FIXED_TIMESTEP = 1.0f / 30.0f; // Reduce to 30 FPS to decrease CPU load
  static constexpr float DAMAGE_COOLDOWN = 1.0f;        // 1 second between damage applications
  float m_accumulator = 0.0f;
  sf::Clock m_clock;

  // Track last damage time for each player
  std::unordered_map<entt::entity, float> m_last_damage_time;

public:
  void suspend()
  {
    if ( m_clock.isRunning() ) m_clock.stop();
  }
  void resume()
  {
    if ( not m_clock.isRunning() ) m_clock.start();
  }
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_WATER_SYSTEM_HPP__