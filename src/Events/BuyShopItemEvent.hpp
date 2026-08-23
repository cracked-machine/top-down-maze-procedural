#ifndef SRC_EVENTS_BUYSHOPITEMEVENT_HPP__
#define SRC_EVENTS_BUYSHOPITEMEVENT_HPP__

namespace Game::Events
{

//! @brief Requests that the shop system sell the player the item at a given slot index.
struct BuyShopItemEvent
{
  //! @brief Index of the item slot in the shop's inventory to purchase.
  uint8_t m_item_idx;
};

} // namespace Game::Events

#endif // SRC_EVENTS_BUYSHOPITEMEVENT_HPP__
