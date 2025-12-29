#ifndef SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP_
#define SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP_

#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events
{

// Event to lock a passage in the system
class PassageEvent
{
public:
  enum class Type {
    REMOVE_PASSAGES,               //< Remove all passages
    OPEN_PASSAGES,                 //< Open all passages
    CONNECT_START_TO_OPENROOMS,    //< Connect passage from the start room to all open rooms
    CONNECT_OCCUPIED_TO_OPENROOMS, //< Connect passage from the occupied room to all open rooms
    CONNECT_OCCUPIED_TO_ENDROOM,   //< Connect passage from the occupied room to the end room
    CONNECT_ALL_OPENROOMS          //< Connect all open rooms with passages
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

} // namespace ProceduralMaze::Events

#endif // SRC_SYSTEMS_EVENTS_PASSAGEEVENT_HPP_