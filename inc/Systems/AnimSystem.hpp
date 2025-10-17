#ifndef INC_SYSTEMS_ANIMSYSTEM_HPP
#define INC_SYSTEMS_ANIMSYSTEM_HPP

#include <BaseSystem.hpp>
#include <Direction.hpp>
#include <Events/AnimDirectionChangeEvent.hpp>
#include <Events/AnimResetFrameEvent.hpp>
#include <SFML/System/Time.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

class AnimSystem : public BaseSystem
{
public:
  AnimSystem( ProceduralMaze::SharedEnttRegistry reg )
      : BaseSystem( reg )
  {
    std::ignore = getEventDispatcher()
                      .sink<Events::AnimResetFrameEvent>()
                      .connect<&Sys::AnimSystem::on_anim_reset_frame>( this );
    std::ignore = getEventDispatcher()
                      .sink<Events::AnimDirectionChangeEvent>()
                      .connect<&Sys::AnimSystem::on_anim_direction_change>( this );
  }

  // update frames on all SpriteAnimation components
  void update( sf::Time deltaTime );

  // event handlers
  void on_anim_reset_frame( const Events::AnimResetFrameEvent &event );
  void on_anim_direction_change( const Events::AnimDirectionChangeEvent &event );

private:
  // Helper function to update a single animation
  void update_frame( Cmp::SpriteAnimation &anim, sf::Time deltaTime, sf::Time frame_rate = sf::seconds( 0.1f ) );
};

} // namespace ProceduralMaze::Sys

#endif // INC_SYSTEMS_ANIMSYSTEM_HPP