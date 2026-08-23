#ifndef SRC_EVENTS_CRYPTROOMEVENT_HPP__
#define SRC_EVENTS_CRYPTROOMEVENT_HPP__

namespace Game::Events
{

//! @brief Signals a change in the crypt's passage/room layout, e.g. reshuffling passages or unlocking exits.
class CryptRoomEvent
{
public:
  //! @brief The kind of crypt room/passage change being requested.
  enum class Type
  {
    //! @brief Randomise which passages lead where.
    SHUFFLE_PASSAGES,
    //! @brief Unlock the passage leading to the crypt's final objective.
    FINAL_PASSAGE,
    //! @brief Unlock all passages leading out of the crypt.
    EXIT_ALL_PASSAGES
  };
  //! @brief The kind of crypt room/passage change requested by this event.
  Type type;
  //! @brief Construct a new CryptRoomEvent object
  //! @param t The kind of crypt room/passage change being requested.
  explicit CryptRoomEvent( Type t )
      : type( t )
  {
  }
};

} // namespace Game::Events

#endif // SRC_EVENTS_CRYPTROOMEVENT_HPP__
