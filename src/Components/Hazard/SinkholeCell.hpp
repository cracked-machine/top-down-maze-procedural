#ifndef SRC_COMPONENTS_HAZARD_SINKHOLECELL_HPP__
#define SRC_COMPONENTS_HAZARD_SINKHOLECELL_HPP__

#include <Components/Hazard/FieldCell.hpp>

namespace Game::Cmp
{

//! @brief A single cell of a sinkhole hazard field. Sinkhole fields instantly kill any entity that falls into them.
struct SinkholeCell : public Hazard::FieldCell
{
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_HAZARD_SINKHOLECELL_HPP__
