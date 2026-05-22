#ifndef SRC_CMP_INVENTORYWEARLEVEL_HPP_
#define SRC_CMP_INVENTORYWEARLEVEL_HPP_

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

#endif // SRC_CMP_INVENTORYWEARLEVEL_HPP_