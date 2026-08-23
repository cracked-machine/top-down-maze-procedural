#ifndef SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__
#define SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__

#include <Components/Position.hpp>
namespace Game::Cmp::Ruin
{

//! @brief Component marking a staircase access point between the ruin's upper and lower floors.
//! @note Colliding with this triggers a floor-change scene event in the direction of #m_direction
//! (see RuinSystem::check_floor_access_collision).
class FloorAccess : public Cmp::Position
{

public:
  //! @brief Direction of floor travel granted by this access point.
  enum class Direction {
    //! @brief Travels down to the ruin's lower floor.
    TO_LOWER,
    //! @brief Travels up to the ruin's upper floor.
    TO_UPPER
  };

  //! @brief Construct a new FloorAccess.
  //! @param pos Grid position, in pixels.
  //! @param size Grid square size, in pixels.
  //! @param direction Direction of floor travel this access point grants.
  FloorAccess( sf::Vector2f pos, sf::Vector2f size, Direction direction )
      : Cmp::Position( pos, size ),
        m_direction( direction )
  {
  }
  //! @brief Direction of floor travel granted by this access point.
  Direction m_direction;
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__
