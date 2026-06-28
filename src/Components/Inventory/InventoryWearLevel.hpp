#ifndef SRC_COMPONENTS_INVENTORY_INVENTORYWEARLEVEL_HPP__
#define SRC_COMPONENTS_INVENTORY_INVENTORYWEARLEVEL_HPP__

namespace Game::Cmp
{

class InventoryWearLevel
{
public:
  InventoryWearLevel( float level )
      : m_level( level )
  {
  }
  float m_level;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_INVENTORYWEARLEVEL_HPP__
