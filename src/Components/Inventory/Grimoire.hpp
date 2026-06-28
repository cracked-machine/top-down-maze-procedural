#ifndef SRC_CMPS_INVENTORY_GRIMOIRE_HPP
#define SRC_CMPS_INVENTORY_GRIMOIRE_HPP

#include <Components/Inventory/WorldItem.hpp>
#include <Systems/Stores/ItemStore.hpp>

namespace Game::Cmp
{

class Grimoire
{
public:
  Grimoire()
  {
    contents.emplace( "item.plant1", false );
    contents.emplace( "item.plant2", false );
    contents.emplace( "item.plant3", false );
    contents.emplace( "item.plant4", false );
    contents.emplace( "item.plant5", false );
    contents.emplace( "item.plant6", false );
  }

  using ItemKey = std::string;
  using Enabled = bool;
  std::map<ItemKey, Enabled> contents;
  bool is_enabled{ false };
};

} // namespace Game::Cmp

#endif // SRC_CMPS_INVENTORY_GRIMOIRE_HPP