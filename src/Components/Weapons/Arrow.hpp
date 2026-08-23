#ifndef SRC_COMPONENTS_ARROWPROJECTILE_HPP__
#define SRC_COMPONENTS_ARROWPROJECTILE_HPP__

#include <SFML/System/Time.hpp>

namespace Game::Cmp::Weapons::Projectiles
{

//! @brief Fired arrow projectile
//! @note User input triggered by SceneInputRouter.
//! @note Arrow physics are managed by ArrowSystem.
class Arrow
{
public:
  //! @brief Construct a new Arrow projectile.
  //! @param origin The position when the arrow was created.
  //! @param destination The mouse pointer position when the arrow was created.
  Arrow( sf::Vector2f origin, sf::Vector2f destination )
      : m_origin( origin ),
        m_destination( destination )
  {
  }

  //! @brief The position when the arrow was created.
  sf::Vector2f m_origin;
  //! @brief  The mouse pointer position when the arrow was created.
  sf::Vector2f m_destination;
  //! @brief The update delay counter
  sf::Time m_fixed_time_step_accumulator{ sf::Time::Zero };

  //! @brief Time elapsed since the arrow landed; drives the post-impact wiggle
  sf::Time m_landed_elapsed{ sf::Time::Zero };
  //! @brief Resting rotation angle (degrees) the wiggle oscillates around
  float m_rest_angle{ 0.f };

  //! @brief Get the speed member
  //! @return float
  [[nodiscard]] float speed() const { return m_speed; }

  //! @brief get the update delay threshold
  [[nodiscard]] sf::Time fixed_time_step_max() const { return m_fixed_time_step_max; }

  //! @brief Peak height (in pixels) of the visual arc between origin and destination
  [[nodiscard]] float arc_height() const { return m_arc_height; }

private:
  //! @brief The speed modifier for the arrow velocity
  float m_speed{ 10.f };

  //! @brief The update delay threshold.
  sf::Time m_fixed_time_step_max{ sf::microseconds( 1.f ) };

  //! @brief The max height arc for the parabola
  float m_arc_height{ 40.f };
};

} // namespace Game::Cmp::Weapons::Projectiles

#endif // SRC_COMPONENTS_ARROWPROJECTILE_HPP__
