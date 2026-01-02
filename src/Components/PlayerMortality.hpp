#ifndef __CMP_PLAYERMORTALITY_HPP__
#define __CMP_PLAYERMORTALITY_HPP__

#include <cstdint>
namespace ProceduralMaze::Cmp
{

class PlayerMortality
{

public:
  enum class State : uint8_t {
    //! @brief Player is alive and well. Game is ongoing.
    ALIVE = 0,
    //! @brief Death in process: Player is falling
    FALLING = 1,
    //! @brief Death in process: Player is decaying
    DECAYING = 2,
    //! @brief Death in process: Player is haunted
    HAUNTED = 3,
    //! @brief Death in process: Player is exploding
    EXPLODING = 4,
    //! @brief Death in process: Player is drowning
    DROWNING = 5, // unimplemented
    SQUISHED = 6, // crypt walls closed on player
    SUICIDE = 7,  // user-specified
    IGNITED = 8,  // lava
    //! @brief Player is dead. final state. end the game.
    DEAD = 255
  };

  explicit PlayerMortality( State initial_state = State::ALIVE, float death_progress_init = 0.0f )
      : state( initial_state ),
        death_progress( death_progress_init )
  {
  }

  State state{ State::ALIVE };

  //! @brief Progress of death animation from 0.0 (start) to 1.0 (complete)
  //! @details Render systems update this value. Logic systems monitor it
  //!          to transition state to DEAD when >= 1.0
  float death_progress{ 0.0f };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_PLAYERMORTALITY_HPP__