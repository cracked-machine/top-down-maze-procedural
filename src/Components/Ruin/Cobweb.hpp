#ifndef SRC_COMPONENTS_RUIN_COBWEB_HPP__
#define SRC_COMPONENTS_RUIN_COBWEB_HPP__

namespace Game::Cmp::Ruin
{

//! @brief Marks an entity as a cobweb obstacle. The player is slowed while colliding with an intact cobweb.
class Cobweb
{
public:
  //! @brief Remaining integrity of the cobweb; the player is only slowed while this is greater than zero.
  int integrity{ 100 };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_COBWEB_HPP__
