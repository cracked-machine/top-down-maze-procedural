#ifndef SRC_COMPONENTS_RUIN_BOOKCASE_HPP__
#define SRC_COMPONENTS_RUIN_BOOKCASE_HPP__

namespace Game::Cmp::Ruin
{

//! @brief Marks an entity as a bookcase obstacle/decoration in the ruin scene.
//! @note Used as a collidable marker when procedurally placing bookcases (see RuinSystem::gen_lowerfloor_bookcases).
class Bookcase
{
public:
  //! @brief Whether this bookcase is currently active (present and blocking).
  bool active{ true };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_BOOKCASE_HPP__
