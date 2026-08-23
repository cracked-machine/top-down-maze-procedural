#ifndef SRC_CMPS_WEAPONS_INFLIGHT_HPP__
#define SRC_CMPS_WEAPONS_INFLIGHT_HPP__

namespace Game::Cmp::Weapons::Projectiles
{

//! @brief Marker component indicating a projectile is currently airborne / in flight.
struct InFlight
{
  //! @brief Whether the projectile is currently in flight.
  [[maybe_unused]] bool active{ true };
};

} // namespace Game::Cmp::Weapons::Projectiles

#endif // SRC_CMPS_WEAPONS_INFLIGHT_HPP__