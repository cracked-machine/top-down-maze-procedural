#ifndef SRC_EVENTS_DROPINVENTORYEVENT_HPP__
#define SRC_EVENTS_DROPINVENTORYEVENT_HPP__

namespace Game::Events
{

//! @brief Requests that the item held in a player inventory slot be dropped into the world.
struct DropInventoryEvent
{
  //! @brief The inventory slot entity whose item should be dropped.
  entt::entity inventory_slot_entt;
  //! @brief World position at which to drop the item.
  sf::Vector2f drop_pos;
};

} // namespace Game::Events

#endif // SRC_EVENTS_DROPINVENTORYEVENT_HPP__
