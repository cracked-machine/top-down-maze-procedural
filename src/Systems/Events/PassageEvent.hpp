#ifndef SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP__
#define SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP__

#include <entt/entity/entity.hpp>

namespace Game::Events
{

//! @brief Event used to trigger PassageSystem operations, such as removing, opening, connecting,
//!        or caching crypt room passages, and adding spike traps.
class PassageEvent
{
public:
  //! @brief The passage operation to perform when this event is handled.
  enum class Type {
    //! @brief Remove all passages
    REMOVE_PASSAGES,
    //! @brief Open all passages
    OPEN_PASSAGES,
    //! @brief Connect passage from the start room to all open rooms
    CONNECT_START_TO_OPENROOMS,
    //! @brief Connect passage from the occupied room to all open rooms
    CONNECT_OCCUPIED_TO_OPENROOMS,
    //! @brief Connect passage from the occupied room to the end room
    CONNECT_OCCUPIED_TO_ENDROOM,
    //! @brief Connect all open rooms with passages
    CONNECT_ALL_ROOMS,
    //! @brief Cache all room-to-room passage connections
    CACHE_ALL_ROOM_CONNECTIONS,
    //! @brief Add spike traps to passages; should be called at the end of the passage creation cycle.
    ADD_SPIKE_TRAPS
  };

  //! @brief Construct a new Passage Event object
  //! @param t Event type
  //! @param e Event entity. Optional
  explicit PassageEvent( Type t, entt::entity e = entt::null )
      : type( t ),
        entity( e )
  {
  }

  //! @brief Event type
  Type type;

  //! @brief Event entity.
  entt::entity entity;
};

} // namespace Game::Events

#endif // SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP__
