#ifndef SRC_COMPONENTS_ARROWPROJECTILE_HPP__
#define SRC_COMPONENTS_ARROWPROJECTILE_HPP__

#include <SFML/System/Time.hpp>
namespace Game::Cmp
{

//! @brief Fired arrow projectile
//! @note User input triggered by SceneInputRouter.
//! @note Arrow physics are managed by PlayerSystem.
//! @note NPC collision detection is managed by NPCSystem.
class ArrowProjectile
{
public:
  ArrowProjectile( sf::Vector2f origin, sf::Vector2f destination )
      : m_origin( origin ),
        m_destination( destination )
  {
  }
  sf::Vector2f m_origin;
  sf::Vector2f m_destination;
  sf::Time m_fixed_time_step_accumulator{ sf::Time::Zero };
  bool m_in_flight{ true };
  //! @brief Time elapsed since the arrow landed; drives the post-impact wiggle
  sf::Time m_landed_elapsed{ sf::Time::Zero };
  //! @brief Resting rotation angle (degrees) the wiggle oscillates around
  float m_rest_angle{ 0.f };

  [[nodiscard]] float speed() const { return m_speed; }
  [[nodiscard]] sf::Time fixed_time_step_max() const { return m_fixed_time_step_max; }
  //! @brief Peak height (in pixels) of the visual arc between origin and destination
  [[nodiscard]] float arc_height() const { return m_arc_height; }

private:
  float m_speed{ 10.f };
  sf::Time m_fixed_time_step_max{ sf::microseconds( 1.f ) };
  float m_arc_height{ 40.f };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ARROWPROJECTILE_HPP__
