#ifndef SRC_COMPONENTS_PLAYER_FOOTSTEPTYPE_HPP__
#define SRC_COMPONENTS_PLAYER_FOOTSTEPTYPE_HPP__

namespace Game::Cmp::Player
{

//! @brief
struct Footstep
{
  //! @brief Which footstep sound effect (if any) should accompany the spawned footstep sprites.
  enum class Type {
    //! @brief No footstep sound effect.
    NONE,
    //! @brief Gravel/outdoor footstep sound effect.
    GRASS,
    //! @brief Wooden floorboard footstep sound effect.
    WOODFLOOR,
    //! @brief Mud footstep sound effect
    MUD,
    //! @brief Stone footstep sound effect
    STONE
  };

  Type m_type = Type::GRASS;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_FOOTSTEPTYPE_HPP__
