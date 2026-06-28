#ifndef SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class RuinProcGenMaxIterations : public BasePersistent<uint16_t>
{
public:
  RuinProcGenMaxIterations( uint16_t value = 0.f, uint16_t min_value = 0.f, uint16_t max_value = 0.f )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "RuinProcGenMaxIterations"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__
