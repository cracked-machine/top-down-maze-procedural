#ifndef SRC_EVENTS_PLAYERACTIONEVENT_HPP__
#define SRC_EVENTS_PLAYERACTIONEVENT_HPP__

namespace Game::Events
{

//! @brief Signals that the player has performed a gameplay action, dispatched to whichever systems care about it.
struct PlayerActionEvent
{
  //! @brief The kind of action the player has performed.
  enum class GameActions
  {
    //! @brief Arm/drop a bomb at the player's position.
    DROP_BOMB,
    //! @brief Arm a bomb placed on a grave.
    GRAVE_BOMB,
    //! @brief Generic interact action, e.g. altars, exits, healing springs, crypt objectives and chests.
    ACTIVATE,
    //! @brief Dig at diggable obstacles, plants, or graves near the player.
    DIG,
    //! @brief Perform a melee (axe) attack.
    ATTACK,
    //! @brief Drop the currently carried inventory item, or pick one up if standing over one.
    DROP_CARRYITEM,
    //! @brief Select a moveable obstacle.
    SELECT,
    //! @brief Deselect the currently selected moveable obstacle.
    DESELECT,
    //! @brief Toggle the grimoire overlay open/closed.
    GRIMOIRE,
    //! @brief Begin drawing/nocking the bow.
    DRAW_BOW,
    //! @brief Release the drawn bow, firing an arrow.
    RELEASE_BOW
  };

  //! @brief Construct a new PlayerActionEvent object
  //! @param act The action being performed.
  //! @param entt Optional entity associated with the action, e.g. the target of a DIG action; defaults to entt::null.
  explicit PlayerActionEvent( GameActions act, entt::entity entt = entt::null )
      : action( act ),
        m_entt( entt )
  {
  }

  //! @brief The action being performed.
  GameActions action;
  //! @brief Optional entity associated with the action; entt::null if not applicable.
  entt::entity m_entt;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PLAYERACTIONEVENT_HPP__
