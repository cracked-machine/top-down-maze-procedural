#ifndef SRC_EVENTS_PLAYERACTIONEVENT_HPP__
#define SRC_EVENTS_PLAYERACTIONEVENT_HPP__

namespace Game::Events
{

struct PlayerActionEvent
{
  enum class GameActions { DROP_BOMB, GRAVE_BOMB, ACTIVATE, DIG, ATTACK, DROP_CARRYITEM, SELECT, DESELECT, GRIMOIRE };

  explicit PlayerActionEvent( GameActions act, entt::entity entt = entt::null )
      : action( act ),
        m_entt( entt )
  {
  }

  GameActions action;
  entt::entity m_entt;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PLAYERACTIONEVENT_HPP__
