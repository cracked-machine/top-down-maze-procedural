#ifndef SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__
#define SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class MaxNumCrypts : public BasePersistent<uint8_t>
{
public:
  MaxNumCrypts( uint8_t value = 3, uint8_t min_value = 1, uint8_t max_value = 10 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MaxNumCrypts"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__
