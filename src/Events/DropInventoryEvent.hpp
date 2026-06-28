#ifndef SRC_EVENTS_DROPINVENTORYEVENT_HPP__
#define SRC_EVENTS_DROPINVENTORYEVENT_HPP__

namespace Game::Events
{

struct DropInventoryEvent
{
  entt::entity inventory_slot_entt;
  sf::Vector2f drop_pos;
};

} // namespace Game::Events

#endif // SRC_EVENTS_DROPINVENTORYEVENT_HPP__
