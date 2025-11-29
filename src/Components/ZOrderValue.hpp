#ifndef SRC_COMPONENTS_ZORDERVALUE_HPP_
#define SRC_COMPONENTS_ZORDERVALUE_HPP_

namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp
#endif // SRC_COMPONENTS_ZORDERVALUE_HPP_