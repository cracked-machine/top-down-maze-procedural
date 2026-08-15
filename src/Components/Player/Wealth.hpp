#ifndef SRC_COMPONENTS_PLAYER_WEALTH_HPP__
#define SRC_COMPONENTS_PLAYER_WEALTH_HPP__

#include <cstdint>
namespace Game::Cmp::Player
{

class Wealth
{
public:
  explicit Wealth( uint32_t initial_wealth = 0 )
      : wealth( initial_wealth )
  {
  }

  // i32 to mitigate accidental underflow as much as possible
  int32_t wealth{ 0 };
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_WEALTH_HPP__
