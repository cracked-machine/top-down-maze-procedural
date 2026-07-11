#ifndef SRC_COMPONENTS_INVENTORY_FLASHUIINVENTORY_HPP__
#define SRC_COMPONENTS_INVENTORY_FLASHUIINVENTORY_HPP__

namespace Game::Cmp
{

//! @brief Signals to the Render system to flash the UI for player inventory
//! @note This is destroyed by the Render system.
struct FlashUIInventory
{
  sf::Time duration{ sf::seconds( 3.f ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_FLASHUIINVENTORY_HPP__
