#ifndef SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__
#define SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__

namespace Game::Cmp
{

//! @brief Signals to the Render system to flash the UI for player bomb radius
//! @note This is destroyed by the Render system.
struct FlashUIRadius
{
  sf::Time duration{ sf::seconds( 3.f ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_FLASHUIRADIUS_HPP__
