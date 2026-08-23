#ifndef SRC_COMPONENTS_INVENTORY_FLASHUIHEALTH_HPP__
#define SRC_COMPONENTS_INVENTORY_FLASHUIHEALTH_HPP__

namespace Game::Cmp
{

//! @brief Signals to the Render system to flash the UI for player health
//! @note This is destroyed by the Render system.
struct FlashUIHealth
{
  //! @brief How long the flash effect lasts before this component is destroyed.
  sf::Time duration{ sf::Time::Zero };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_FLASHUIHEALTH_HPP__
