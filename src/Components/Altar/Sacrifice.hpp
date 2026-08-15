#ifndef SRC_COMPONENTS_ALTAR_SACRIFICE_HPP__
#define SRC_COMPONENTS_ALTAR_SACRIFICE_HPP__

namespace Game::Cmp::Altar
{

//! @brief Marker component for identifying in-progress altar sacrifice animations
struct Sacrifice
{
  bool active{ true };
};

} // namespace Game::Cmp::Altar

#endif // SRC_COMPONENTS_ALTAR_SACRIFICE_HPP__
