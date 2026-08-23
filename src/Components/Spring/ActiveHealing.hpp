#ifndef SRC_COMPONENTS_HEALINGSPRING_ACTIVEHEALING_HPP__
#define SRC_COMPONENTS_HEALINGSPRING_ACTIVEHEALING_HPP__

namespace Game::Cmp::HealingSpring
{

//! @brief Tag component attached to a healing spring/fountain entity while it is actively healing the player.
struct ActiveHealing
{
  //! @brief Whether healing is currently active. Presence of the component itself is also used as a tag by callers.
  [[maybe_unused]] bool active{ true };
};

} // namespace Game::Cmp::HealingSpring

#endif // SRC_COMPONENTS_HEALINGSPRING_ACTIVEHEALING_HPP__
