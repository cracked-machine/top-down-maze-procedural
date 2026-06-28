#ifndef SRC_COMPONENTS_ALTAR_ALTARSACRIFICE_HPP__
#define SRC_COMPONENTS_ALTAR_ALTARSACRIFICE_HPP__

namespace Game::Cmp
{

//! @brief Marker component for identifying in-progress altar sacrifice animations
struct AltarSacrifice
{
  bool active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ALTAR_ALTARSACRIFICE_HPP__
