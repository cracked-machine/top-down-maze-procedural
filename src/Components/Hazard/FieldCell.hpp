#ifndef SRC_COMPONENTS_HAZARD_FIELDCELL_HPP__
#define SRC_COMPONENTS_HAZARD_FIELDCELL_HPP__

namespace Game::Cmp::Hazard
{

struct FieldCell
{
  //! @brief polymorphic destructor for derived classes
  virtual ~FieldCell() = default;

  // active hazard fields can cause adjacent obstacles to become hazard fields
  // inactive hazard fields are inner hazard fields and can be skipped by the spread algorithm
  bool active{ true };
};

} // namespace Game::Cmp::Hazard

#endif // SRC_COMPONENTS_HAZARD_FIELDCELL_HPP__
