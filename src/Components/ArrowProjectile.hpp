#ifndef SRC_COMPONENTS_ARROWPROJECTILE_HPP__
#define SRC_COMPONENTS_ARROWPROJECTILE_HPP__

#include <SFML/System/Time.hpp>
namespace Game::Cmp
{

//! @brief Fired arrow projectile
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

  [[nodiscard]] float speed() const { return m_speed; }
  [[nodiscard]] sf::Time fixed_time_step_max() const { return m_fixed_time_step_max; }

private:
  float m_speed{ 10.f };
  sf::Time m_fixed_time_step_max{ sf::microseconds( 1.f ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ARROWPROJECTILE_HPP__
