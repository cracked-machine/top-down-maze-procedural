#ifndef SRC_EVENTS_PICKUPWORLDITEMEVENT_HPP__
#define SRC_EVENTS_PICKUPWORLDITEMEVENT_HPP__

namespace Game::Events
{

//! @brief Requests that a world item entity be picked up into a new player inventory slot.
struct PickupWorldItemEvent
{
  //! @brief The world item entity to pick up.
  entt::entity world_item_entt;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PICKUPWORLDITEMEVENT_HPP__
