#ifndef SRC_COMPONENTS_HAZARD_CORRUPTIONCELL_HPP__
#define SRC_COMPONENTS_HAZARD_CORRUPTIONCELL_HPP__

#include <Components/Hazard/FieldCell.hpp>

namespace Game::Cmp
{

//! @brief A single cell of a corruption hazard field. Corruption fields are composed of many CorruptionCell
//! entities and gradually drain health from entities standing in them over time.
struct CorruptionCell : public Hazard::FieldCell
{
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_HAZARD_CORRUPTIONCELL_HPP__
