#ifndef SRC_CMPS_SHOP_SHOPINVENTORY_HPP__
#define SRC_CMPS_SHOP_SHOPINVENTORY_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/Constants.hpp>

namespace Game::Cmp
{

class ShopInventory
{
public:
  struct Config
  {
    int max_items;
    int min_price;
    int max_price;
  };

  ShopInventory() = default;

  std::vector<std::pair<Sprites::SpriteMetaType, int32_t>> m_slots;

  Config m_config;

  bool is_enabled{ false };
};

} // namespace Game::Cmp

#endif // SRC_CMPS_SHOP_SHOPINVENTORY_HPP__