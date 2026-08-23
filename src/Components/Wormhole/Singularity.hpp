#ifndef SRC_COMPONENTS_WORMHOLE_SINGULARITY_HPP__
#define SRC_COMPONENTS_WORMHOLE_SINGULARITY_HPP__

namespace Game::Cmp::Wormhole
{

//! @brief Marker component identifying the singularity (central point) entity of a wormhole.
class Singularity
{
public:
  //! @brief Whether the singularity is currently active.
  bool active{ true };
};

} // namespace Game::Cmp::Wormhole

#endif // SRC_COMPONENTS_WORMHOLE_SINGULARITY_HPP__
