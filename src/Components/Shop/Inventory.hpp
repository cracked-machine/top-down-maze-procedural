#ifndef SRC_COMPONENTS_SHOP_INVENTORY_HPP__
#define SRC_COMPONENTS_SHOP_INVENTORY_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/Constants.hpp>

namespace Game::Cmp::Shop
{

class Inventory
{
public:
  struct Config
  {
    int max_items;
    int min_price;
    int max_price;
  };

  Inventory() = default;

  std::vector<std::pair<Sprites::SpriteMetaType, int32_t>> m_slots;

  Config m_config;

  bool is_enabled{ false };
};

} // namespace Game::Cmp::Shop

#endif // SRC_COMPONENTS_SHOP_INVENTORY_HPP__
