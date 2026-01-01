#ifndef SRC_SYSTEMS_THREATS_SHOCKWAVESYSTEM_HPP_
#define SRC_SYSTEMS_THREATS_SHOCKWAVESYSTEM_HPP_

#include <Components/NpcShockwave.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/Shockwave.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class ShockwaveSystem : public BaseSystem
{
public:
  ShockwaveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  // Collision detection utilities
  static bool pointIntersectsVisibleSegments( const Cmp::NpcShockwave &shockwave, sf::Vector2f point );
  static bool intersectsWithVisibleSegments( const Cmp::NpcShockwave &shockwave, const sf::FloatRect &rect );

  // Segment manipulation utilities
  static void removeIntersectingSegments( const sf::FloatRect &obstacle_rect, Cmp::NpcShockwave &shockwave );
  static Sprites::Shockwave::CircleSegments splitSegmentByObstacle( const Sprites::CircleSegment &segment, const sf::FloatRect &obstacle_rect,
                                                                    sf::Vector2f shockwave_position, float radius, const int samples );

  //! @brief event handlers for pausing system clocks
  void onPause() override {};
  //! @brief event handlers for resuming system clocks
  void onResume() override {};
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_THREATS_SHOCKWAVESYSTEM_HPP_