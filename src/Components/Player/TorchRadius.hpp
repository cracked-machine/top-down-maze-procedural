#ifndef SRC_COMPONENTS_PLAYER_TORCHRADIUS_HPP__
#define SRC_COMPONENTS_PLAYER_TORCHRADIUS_HPP__

namespace Game::Cmp
{

//! @brief Radius of light cast by the player's torch, used by the night/vision shader.
struct TorchRadius
{
  //! @brief Torch light radius.
  float value;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYER_TORCHRADIUS_HPP__
