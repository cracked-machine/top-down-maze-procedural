#ifndef SRC_COMPONENTS_INVENTORY_FLASHUICADAVER_HPP__
#define SRC_COMPONENTS_INVENTORY_FLASHUICADAVER_HPP__

namespace Game::Cmp
{

//! @brief Signals to the Render system to flash the UI for player cadaver count
//! @note This is destroyed by the Render system.
struct FlashUICadaver
{
  //! @brief How long the flash effect lasts before this component is destroyed.
  sf::Time duration{ sf::seconds( 3.f ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_FLASHUICADAVER_HPP__
