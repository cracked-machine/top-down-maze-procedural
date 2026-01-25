#ifndef __CMP_HAZARDFIELDCELL_HPP__
#define __CMP_HAZARDFIELDCELL_HPP__

namespace ProceduralMaze::Cmp {

struct HazardFieldCell
{
  //! @brief polymorphic destructor for derived classes
  virtual ~HazardFieldCell() = default;

  // active hazard fields can cause adjacent obstacles to become hazard fields
  // inactive hazard fields are inner hazard fields and can be skipped by the spread algorithm
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_HAZARDFIELDCELL_HPP__