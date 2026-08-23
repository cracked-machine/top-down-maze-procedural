#ifndef SRC_COMPONENTS_PLAYER_WEALTH_HPP__
#define SRC_COMPONENTS_PLAYER_WEALTH_HPP__

#include <cstdint>
namespace Game::Cmp::Player
{

//! @brief Tracks the amount of currency/wealth the player holds.
class Wealth
{
public:
  //! @brief Construct with an initial wealth value.
  //! @param initial_wealth initial wealth value
  explicit Wealth( uint32_t initial_wealth = 0 )
      : wealth( initial_wealth )
  {
  }

  //! @brief Current wealth value.
  //! @note Stored as int32_t (rather than unsigned) to mitigate accidental underflow as much as possible.
  int32_t wealth{ 0 };
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_WEALTH_HPP__
