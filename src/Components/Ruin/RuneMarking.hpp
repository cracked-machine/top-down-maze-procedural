#ifndef SRC_COMPONENTS_RUIN_RUNEMARKING_HPP__
#define SRC_COMPONENTS_RUIN_RUNEMARKING_HPP__

namespace Game::Cmp
{

//! @brief Marks a rune-marking floor tile used by the ruin's lower-floor puzzle.
//! @note The puzzle is solved once every RuneMarking's #active is true (see RuinSystem::check_puzzle_status),
//! which unlocks the Cmp::Ruin::StairsGateMultiBlock gate. #active is set when an obstacle is placed on the marking.
class RuneMarking
{
public:
  //! @brief Whether this rune marking is currently activated (an obstacle is placed on it).
  bool active{ false };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RUIN_RUNEMARKING_HPP__
