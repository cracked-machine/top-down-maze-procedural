#ifndef INC_SYSTEMS_ANIMSYSTEM_HPP
#define INC_SYSTEMS_ANIMSYSTEM_HPP

#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/System/Time.hpp>

#include <Components/Direction.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class AnimSystem : public BaseSystem
{
public:
  AnimSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
    // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
    std::ignore = getEventDispatcher().sink<Events::PauseClocksEvent>().connect<&Sys::AnimSystem::onPause>( this );
    std::ignore = getEventDispatcher().sink<Events::ResumeClocksEvent>().connect<&Sys::AnimSystem::onResume>( this );
    SPDLOG_DEBUG( "AnimSystem initialized" );
  }

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // update frames on all SpriteAnimation components
  void update( sf::Time globalDeltaTime );

private:
  //! @brief Update a single animation sequence. Used for tilesets that consist of a single animation sequence.
  //! @param anim
  //! @param globalDeltaTime
  //! @param frame_rate
  void update_single_sequence( Cmp::SpriteAnimation &anim, sf::Time globalDeltaTime, const Sprites::MultiSprite &ms,
                               sf::Time frame_rate );

  //! @brief Update grouped animation sequences. Used for tilesets that consist of multiple direction sequences.
  //! @param anim
  //! @param globalDeltaTime
  //! @param frame_rate
  void update_grouped_sequences( Cmp::SpriteAnimation &anim, sf::Time globalDeltaTime, const Sprites::MultiSprite &,
                                 sf::Time frame_rate );
};

} // namespace ProceduralMaze::Sys

#endif // INC_SYSTEMS_ANIMSYSTEM_HPP