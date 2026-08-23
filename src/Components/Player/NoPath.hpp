#ifndef SRC_COMPONENTS_PLAYER_NOPATH_HPP__
#define SRC_COMPONENTS_PLAYER_NOPATH_HPP__

namespace Game::Cmp::Player
{

//! @brief Mark entity that blocks player movement
struct NoPath
{
  //! @brief Whether this entity currently blocks player movement.
  //! @details This is used by `PlayerSystem::is_valid_move` for collision detection.
  //!        If set to false then collision detection for this entity is disabled.
  //!        `Cmp::PlantSegments` automatically set this to false and is reactivated in
  //!        `PlayerSystem::update_player_no_path_cmp` when player vacates its hitbox.
  bool active{ true };
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_NOPATH_HPP__
