#ifndef SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__
#define SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__

namespace Game::Cmp
{

//! @brief Signals to the Render system to flash the UI for player bomb radius
//! @note This is destroyed by the Render system.
struct FlashUIRadius
{
  //! @brief How long the flash effect lasts before this component is destroyed.
  sf::Time duration{ sf::seconds( 3.f ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__
