#ifndef SRC_COMPONENTS_PERSISTENT_BLASTRADIUS_HPP__
#define SRC_COMPONENTS_PERSISTENT_BLASTRADIUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>
namespace Game::Cmp::Persist
{

//! @brief Default blast radius (in grid tiles) applied to the player's bombs.
class BlastRadius : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct with an initial blast radius and optional min/max bounds.
  //! @param value initial blast radius
  //! @param min_value minimum allowed blast radius
  //! @param max_value maximum allowed blast radius
  BlastRadius( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "BlastRadius"
  virtual std::string class_name() const override { return "BlastRadius"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_BLASTRADIUS_HPP__
