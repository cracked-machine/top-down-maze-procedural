#ifndef SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__
#define SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class MaxNumCrypts : public BasePersistent<uint8_t>
{
public:
  MaxNumCrypts( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MaxNumCrypts"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMP_PERSISTENT_MAXNUMCRYPTS_HPP__
