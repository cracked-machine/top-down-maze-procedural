#ifndef SRC_EVENTS_DROPINVENTORYEVENT_
#define SRC_EVENTS_DROPINVENTORYEVENT_

namespace ProceduralMaze::Events
{

struct DropInventoryEvent
{
  entt::entity inventory_slot_entt;
  sf::Vector2f drop_pos;
};

} // namespace ProceduralMaze::Events

#endif // SRC_EVENTS_DROPINVENTORYEVENT_