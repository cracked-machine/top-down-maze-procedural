#ifndef SRC_EVENTS_CRYPTROOMEVENT_HPP_
#define SRC_EVENTS_CRYPTROOMEVENT_HPP_

namespace ProceduralMaze::Events
{

// Event to lock a crypt room in the system
class CryptRoomEvent
{
public:
  enum class Type { SHUFFLE_PASSAGES, FINAL_PASSAGE, EXIT_ALL_PASSAGES };
  Type type;
  explicit CryptRoomEvent( Type t )
      : type( t )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // SRC_EVENTS_CRYPTROOMEVENT_HPP_