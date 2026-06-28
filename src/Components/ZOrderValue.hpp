#ifndef SRC_COMPONENTS_ZORDERVALUE_HPP__
#define SRC_COMPONENTS_ZORDERVALUE_HPP__

namespace Game::Cmp
{

class ZOrderValue
{
public:
  ZOrderValue( float z )
      : zOrder( z ) {};

  float getZOrder() const { return zOrder; }
  void setZOrder( float z ) { zOrder = z; }

private:
  float zOrder;
};

} // namespace Game::Cmp
#endif // SRC_COMPONENTS_ZORDERVALUE_HPP__
