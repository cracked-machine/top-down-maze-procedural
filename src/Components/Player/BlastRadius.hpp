#ifndef SRC_COMPONENTS_PLAYER_BLASTRADIUS_HPP__
#define SRC_COMPONENTS_PLAYER_BLASTRADIUS_HPP__

namespace Game::Cmp::Player
{

//! @brief Radius (in tiles) of the player's bomb blast, mirrored from Cmp::Persist::BlastRadius.
struct BlastRadius
{
  //! @brief Blast radius value.
  int value{};
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_BLASTRADIUS_HPP__
