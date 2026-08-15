#ifndef SRC_COMPONENTS_INVENTORY_WEARLEVEL_HPP__
#define SRC_COMPONENTS_INVENTORY_WEARLEVEL_HPP__

namespace Game::Cmp::Inventory
{

class WearLevel
{
public:
  WearLevel( float level )
      : m_level( level )
  {
  }
  float m_level;
};

} // namespace Game::Cmp::Inventory

#endif // SRC_COMPONENTS_INVENTORY_WEARLEVEL_HPP__
